import subprocess
import os
import json
import csv
import psycopg2


def execute(cmd, memcheck=False):
    if memcheck:
        valgrind = [
            "libtool",
            "--mode=execute",
            "valgrind",
            "--leak-check=full",
            "--show-leak-kinds=all",
            "--error-exitcode=42",
        ]
        cmd = valgrind + cmd

    print(f"Executing command: '{' '.join(cmd)}' from '{os.getcwd()}'")
    proc = subprocess.run(cmd, capture_output=True)
    print(proc.stdout.decode(errors="ignore"), end="")
    print(proc.stderr.decode(errors="ignore"), end="")
    print(f"Command returned {proc.returncode}")
    return proc.returncode


def test_leech_csv_delta(tmp_path):
    ##########################################################################
    # Create config
    ##########################################################################

    bin_path = os.path.join("bin", "leech")
    leech_conf_path = os.path.join(tmp_path, "leech.json")
    table_src_path = os.path.join(tmp_path, "beatles.src.csv")
    table_dst_path = os.path.join(tmp_path, "beatles.dst.csv")

    config = {
        "version": "0.1.0",
        "tables": {
            "BTL": {
                "primary_fields": ["first_name", "last_name"],
                "subsidiary_fields": ["born"],
                "source": {
                    "params": table_src_path,
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
                "destination": {
                    "params": table_dst_path,
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
            }
        },
    }
    with open(leech_conf_path, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Created leech config '{leech_conf_path}' with content:")
    with open(leech_conf_path, "r") as f:
        print(f.read())

    ##########################################################################
    # Create table and commit
    ##########################################################################

    table = [
        ["first_name", "last_name", "born"],
        ["Paul", "McCartney", "1942"],
        ["Ringo", "Starr", "1940"],
        ["John", "Lennon", "1940"],
        ["George", "Harrison", "1943"],
    ]
    with open(table_src_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(table)
    print(f"Created table '{table_src_path}' with content:")
    with open(table_src_path, "r") as f:
        print(f.read())

    command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
    assert execute(command, True) == 0

    ##########################################################################
    # Modify table (with one insert, delete and update) followed by a commit
    ##########################################################################

    table = [
        ["first_name", "last_name", "born"],
        ["Paul", "McCartney", "1943"],
        ["John", "Lennon", "1940"],
        ["George", "Harrison", "1943"],
        ["Janis", "Joplin", "1943"],
    ]
    with open(table_src_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(table)
    print(f"Modified table '{table_src_path}' with content:")
    with open(table_src_path, "r") as f:
        print(f.read())

    command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
    assert execute(command, True) == 0

    ##########################################################################
    # Create delta patch file
    ##########################################################################

    lastknown = "0000000000000000000000000000000000000000"
    patchfile = os.path.join(tmp_path, "patchfile")
    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "diff",
        f"--block={lastknown}",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Apply delta patch file
    ##########################################################################

    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "patch",
        "--field=host_id",
        "--value=SHA=123",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0


def test_leech_psql_delta(tmp_path):
    db_src_name = "src_leech"
    db_dst_name = "dst_leech"

    ##########################################################################
    # Setup test database leech
    ##########################################################################

    execute(["dropdb", "--if-exists", db_src_name])
    execute(["createdb", db_src_name])
    execute(["dropdb", "--if-exists", db_dst_name])
    execute(["createdb", db_dst_name])

    ##########################################################################
    # Create config
    ##########################################################################

    bin_path = os.path.join("bin", "leech")
    leech_conf_path = os.path.join(tmp_path, "leech.json")

    config = {
        "version": "0.1.0",
        "tables": {
            "BTL": {
                "primary_fields": ["first_name", "last_name"],
                "subsidiary_fields": ["born"],
                "source": {
                    "params": f"dbname={db_src_name}",
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_psql.so",
                },
                "destination": {
                    "params": f"dbname={db_dst_name}",
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_psql.so",
                },
            }
        },
    }
    with open(leech_conf_path, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Created leech config '{leech_conf_path}' with content:")
    with open(leech_conf_path, "r") as f:
        print(f.read())

    ##########################################################################
    # Create table and commit
    ##########################################################################

    conn = psycopg2.connect(f"dbname={db_src_name}")
    cur = conn.cursor()
    cur.execute(
        """CREATE TABLE beatles (
           first_name TEXT NOT NULL,
           last_name TEXT NOT NULL,
           born TEXT,
           PRIMARY KEY(first_name, last_name));"""
    )

    table = [
        ["Paul", "McCartney", "1942"],
        ["Ringo", "Starr", "1940"],
        ["John", "Lennon", "1940"],
        ["George", "Harrison", "1943"],
    ]
    for record in table:
        first_name, last_name, born = record
        cur.execute(
            "INSERT INTO beatles (first_name, last_name, born) VALUES (%s, %s, %s);",
            (first_name, last_name, born),
        )

    cur.close()
    conn.commit()
    conn.close()

    command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
    assert execute(command, True) == 0

    ##########################################################################
    # Modify table (with one insert, delete and update) followed by a commit
    ##########################################################################

    conn = psycopg2.connect(f"dbname={db_src_name}")
    cur = conn.cursor()
    cur.execute(
        """UPDATE beatles
           SET born = 1993
           WHERE first_name = 'Paul' AND last_name = 'McCartney';"""
    )
    cur.execute(
        """DELETE FROM beatles
           WHERE first_name = 'Ringo' AND last_name = 'Starr';"""
    )
    cur.execute(
        """INSERT INTO beatles
           (first_name, last_name, born) VALUES ('Janis', 'Joplin', '1943');"""
    )

    cur.close()
    conn.commit()
    conn.close()

    command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
    assert execute(command, True) == 0

    ##########################################################################
    # Create patch file
    ##########################################################################

    lastknown = "0000000000000000000000000000000000000000"
    patchfile = os.path.join(tmp_path, "patchfile")
    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "diff",
        f"--block={lastknown}",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Apply delta patch file
    ##########################################################################

    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "patch",
        "--field=host_id",
        "--value=SHA=123",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0


def test_leech_csv_rebase(tmp_path):
    ##########################################################################
    # Create config
    ##########################################################################

    bin_path = os.path.join("bin", "leech")
    leech_conf_path = os.path.join(tmp_path, "leech.json")
    table_src_path = os.path.join(tmp_path, "beatles.src.csv")
    table_dst_path = os.path.join(tmp_path, "beatles.dst.csv")

    config = {
        "version": "0.1.0",
        "tables": {
            "BTL": {
                "primary_fields": ["first_name", "last_name"],
                "subsidiary_fields": ["born"],
                "source": {
                    "params": table_src_path,
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
                "destination": {
                    "params": table_dst_path,
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
            }
        },
    }
    with open(leech_conf_path, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Created leech config '{leech_conf_path}' with content:")
    with open(leech_conf_path, "r") as f:
        print(f.read())

    ##########################################################################
    # Create destination table
    ##########################################################################

    table = [
        ["host_id", "first_name", "last_name", "born"],
        ["SHA=123", "Paul", "McCartney", "1942"],
        ["SHA=123", "Ringo", "Starr", "1940"],
        ["SHA=123", "John", "Lennon", "1940"],
        ["SHA=123", "George", "Harrison", "1943"],
        ["SHA=456", "Paul", "McCartney", "1943"],
        ["SHA=456", "John", "Lennon", "1940"],
        ["SHA=456", "George", "Harrison", "1943"],
        ["SHA=456", "Janis", "Joplin", "1943"],
    ]
    with open(table_dst_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(table)
    print(f"Created table '{table_dst_path}' with content:")
    with open(table_dst_path, "r") as f:
        print(f.read())

    ##########################################################################
    # Create source table and commit
    ##########################################################################

    table = [
        ["first_name", "last_name", "born"],
        ["Beyonce Gisellle", "Knowles", "1981"],
        ["Eric Patrick", "Clapton", "1945"],
        ["Ringo", "Starr", "1940"],
        ["John", "Lennon", "1940"],
        ["George", "Harrison", "1943"],
    ]
    with open(table_src_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(table)
    print(f"Created table '{table_src_path}' with content:")
    with open(table_src_path, "r") as f:
        print(f.read())

    command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
    assert execute(command, True) == 0

    ##########################################################################
    # Create rebase patch
    ##########################################################################

    patchfile = os.path.join(tmp_path, "patchfile")
    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "rebase",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Apply rebase patch file
    ##########################################################################

    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "patch",
        "--field=host_id",
        "--value=SHA=123",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Check the results
    ##########################################################################

    expected = [
        ["host_id", "first_name", "last_name", "born"],
        ["SHA=123", "Beyonce Gisellle", "Knowles", "1981"],
        ["SHA=123", "Eric Patrick", "Clapton", "1945"],
        ["SHA=123", "Ringo", "Starr", "1940"],
        ["SHA=123", "John", "Lennon", "1940"],
        ["SHA=123", "George", "Harrison", "1943"],
        ["SHA=456", "Paul", "McCartney", "1943"],
        ["SHA=456", "John", "Lennon", "1940"],
        ["SHA=456", "George", "Harrison", "1943"],
        ["SHA=456", "Janis", "Joplin", "1943"],
    ]

    with open(table_dst_path, "r", newline="") as f:
        reader = csv.reader(f)
        for row in reader:
            print(row)
            assert any(
                x == row for x in expected
            ), f"Row '{row}' does not match any of the expected"


def test_leech_psql_rebase(tmp_path):
    db_src_name = "src_leech"
    db_dst_name = "dst_leech"

    ##########################################################################
    # Setup test database leech
    ##########################################################################

    execute(["dropdb", "--if-exists", db_src_name])
    execute(["createdb", db_src_name])
    execute(["dropdb", "--if-exists", db_dst_name])
    execute(["createdb", db_dst_name])

    ##########################################################################
    # Create config
    ##########################################################################

    bin_path = os.path.join("bin", "leech")
    leech_conf_path = os.path.join(tmp_path, "leech.json")

    config = {
        "version": "0.1.0",
        "tables": {
            "BTL": {
                "primary_fields": ["first_name", "last_name"],
                "subsidiary_fields": ["born"],
                "source": {
                    "params": f"dbname={db_src_name}",
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_psql.so",
                },
                "destination": {
                    "params": f"dbname={db_dst_name}",
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_psql.so",
                },
            }
        },
    }
    with open(leech_conf_path, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Created leech config '{leech_conf_path}' with content:")
    with open(leech_conf_path, "r") as f:
        print(f.read())

    ##########################################################################
    # Create destination table
    ##########################################################################

    conn = psycopg2.connect(f"dbname={db_dst_name}")
    cur = conn.cursor()
    cur.execute(
        """CREATE TABLE beatles (
           host_id TEXT NOT NULL,
           first_name TEXT NOT NULL,
           last_name TEXT NOT NULL,
           born TEXT,
           PRIMARY KEY(host_id, first_name, last_name));"""
    )

    table = [
        ["SHA=123", "Paul", "McCartney", "1942"],
        ["SHA=123", "Ringo", "Starr", "1940"],
        ["SHA=123", "John", "Lennon", "1940"],
        ["SHA=123", "George", "Harrison", "1943"],
        ["SHA=456", "Paul", "McCartney", "1943"],
        ["SHA=456", "John", "Lennon", "1940"],
        ["SHA=456", "George", "Harrison", "1943"],
        ["SHA=456", "Janis", "Joplin", "1943"],
    ]

    for record in table:
        host_id, first_name, last_name, born = record
        cur.execute(
            "INSERT INTO beatles (host_id, first_name, last_name, born) VALUES (%s, %s, %s, %s);",
            (host_id, first_name, last_name, born),
        )

    cur.close()
    conn.commit()
    conn.close()

    ##########################################################################
    # Create source table and commit
    ##########################################################################

    conn = psycopg2.connect(f"dbname={db_src_name}")
    cur = conn.cursor()
    cur.execute(
        """CREATE TABLE beatles (
           first_name TEXT NOT NULL,
           last_name TEXT NOT NULL,
           born TEXT,
           PRIMARY KEY(first_name, last_name));"""
    )

    table = [
        ["Beyonce Gisellle", "Knowles", "1981"],
        ["Eric Patrick", "Clapton", "1945"],
        ["Ringo", "Starr", "1940"],
        ["John", "Lennon", "1940"],
        ["George", "Harrison", "1943"],
    ]
    for record in table:
        first_name, last_name, born = record
        cur.execute(
            "INSERT INTO beatles (first_name, last_name, born) VALUES (%s, %s, %s);",
            (first_name, last_name, born),
        )

    cur.close()
    conn.commit()
    conn.close()

    command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
    assert execute(command, True) == 0

    ##########################################################################
    # Create rebase patch
    ##########################################################################

    patchfile = os.path.join(tmp_path, "patchfile")
    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "rebase",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Apply rebase patch file
    ##########################################################################

    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "patch",
        "--field=host_id",
        "--value=SHA=123",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Check the results
    ##########################################################################

    expected = [
        ("SHA=123", "Beyonce Gisellle", "Knowles", "1981"),
        ("SHA=123", "Eric Patrick", "Clapton", "1945"),
        ("SHA=123", "Ringo", "Starr", "1940"),
        ("SHA=123", "John", "Lennon", "1940"),
        ("SHA=123", "George", "Harrison", "1943"),
        ("SHA=456", "Paul", "McCartney", "1943"),
        ("SHA=456", "John", "Lennon", "1940"),
        ("SHA=456", "George", "Harrison", "1943"),
        ("SHA=456", "Janis", "Joplin", "1943"),
    ]

    conn = psycopg2.connect(f"dbname={db_dst_name}")
    cur = conn.cursor()

    cur.execute("SELECT host_id, first_name, last_name, born FROM beatles;")
    result = cur.fetchall()

    for row in result:
        print(row)
        assert any(
            x == row for x in expected
        ), f"Row '{row}' does not match any of the expected"

    cur.close()
    conn.close()


def test_leech_csv_binary(tmp_path):
    ##########################################################################
    # Create config
    ##########################################################################

    bin_path = os.path.join("bin", "leech")
    leech_conf_path = os.path.join(tmp_path, "leech.json")
    table_src_path = os.path.join(tmp_path, "beatles.src.csv")
    table_dst_path = os.path.join(tmp_path, "beatles.dst.csv")

    config = {
        "version": "0.1.0",
        "tables": {
            "BTL": {
                "primary_fields": ["first_name", "last_name"],
                "subsidiary_fields": ["born"],
                "source": {
                    "params": table_src_path,
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
                "destination": {
                    "params": table_dst_path,
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
            }
        },
    }
    with open(leech_conf_path, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Created leech config '{leech_conf_path}' with content:")
    with open(leech_conf_path, "r") as f:
        print(f.read())

    ##########################################################################
    # Create table and commit
    ##########################################################################

    with open(table_src_path, "wb") as f:
        f.write(
            b"first_name,last_name,born\r\n"
            + b'"\x00\x01\x02\x03","\x04\x05\x06\x07","\x08\x09\x0A\x0B"\r\n'
            + b'"\x0C\x0D\x0E\x0F","\x10\x11\x12\x13","\x14\x15\x16\x17"\r\n'
            + b'"\x18\x19\x1A\x1B","\x1C\x1D\x1E\x1F","\x20\x21\x22\x22\x23"\r\n'
            + b'"\x24\x25\x26\x27","\x28\x29\x2A\x2B","\x2C\x2D\x2E\x2F"\r\n'
        )
    print(f"Created table '{table_src_path}' with content:")
    with open(table_src_path, "rb") as f:
        print(f.read())

    command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
    assert execute(command, True) == 0

    ##########################################################################
    # Create delta patch file
    ##########################################################################

    lastknown = "0000000000000000000000000000000000000000"
    patchfile = os.path.join(tmp_path, "patchfile")
    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "diff",
        f"--block={lastknown}",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Apply delta patch file
    ##########################################################################

    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "patch",
        "--field=host_id",
        "--value=SHA=123",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Modify table and commit
    ##########################################################################

    with open(table_src_path, "wb") as f:
        f.write(
            b"first_name,last_name,born\r\n"
            + b'"\x00\x01\x02\x03","\x04\x05\x06\x07","\x08\x09\x0A\x0B"\r\n'
            + b'"\x0C\x0D\x0E\x0F","\x10\x11\x12\x13","\x14\x15\x16\x17"\r\n'
            + b'"\x18\x19\x1A\x1B","\x1C\x1D\x1E\x1F","\x30\x31\x32\x33"\r\n'
            + b'"\x34\x35\x36\x37","\x38\x39\x3A\x3B","\x3C\x3D\x3E\x3F"\r\n'
        )
    print(f"Created table '{table_src_path}' with content:")
    with open(table_src_path, "rb") as f:
        print(f.read())

    command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
    assert execute(command, True) == 0

    ##########################################################################
    # Create delta patch file
    ##########################################################################

    with open(os.path.join(tmp_path, "SHA=123"), "r") as f:
        lastknown = f.read().strip()
    patchfile = os.path.join(tmp_path, "patchfile")
    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "diff",
        f"--block={lastknown}",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Apply delta patch file
    ##########################################################################

    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "patch",
        "--field=host_id",
        "--value=SHA=123",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0


def test_leech_garbage_collect(tmp_path):
    ##########################################################################
    # Create config
    ##########################################################################

    bin_path = os.path.join("bin", "leech")
    leech_conf_path = os.path.join(tmp_path, "leech.json")
    table_src_path = os.path.join(tmp_path, "beatles.src.csv")
    table_dst_path = os.path.join(tmp_path, "beatles.dst.csv")

    config = {
        "version": "0.1.0",
        "max_chain_length": 3,
        "tables": {
            "BTL": {
                "primary_fields": ["first_name", "last_name"],
                "subsidiary_fields": ["born"],
                "source": {
                    "params": table_src_path,
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
                "destination": {
                    "params": table_dst_path,
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
            }
        },
    }
    with open(leech_conf_path, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Created leech config '{leech_conf_path}' with content:")
    with open(leech_conf_path, "r") as f:
        print(f.read())

    for _ in range(5):
        command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
        assert execute(command, True) == 0


def test_leech_churn(tmp_path):
    pass
    ##########################################################################
    # Create config
    ##########################################################################

    bin_path = os.path.join("bin", "leech")
    leech_conf_path = os.path.join(tmp_path, "leech.json")
    btl_src_path = os.path.join(tmp_path, "beatles.src.csv")
    btl_dst_path = os.path.join(tmp_path, "beatles.dst.csv")
    pfl_src_path = os.path.join(tmp_path, "pinkfloyd.src.csv")
    pfl_dst_path = os.path.join(tmp_path, "pinkfloyd.dst.csv")

    config = {
        "version": "0.1.0",
        "pretty_print": True,
        "tables": {
            "BTL": {
                "primary_fields": ["first_name", "last_name"],
                "subsidiary_fields": ["born"],
                "source": {
                    "params": btl_src_path,
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
                "destination": {
                    "params": btl_dst_path,
                    "schema": "leech",
                    "table_name": "beatles",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
            },
            "PFL": {
                "primary_fields": ["first_name", "last_name"],
                "subsidiary_fields": ["born"],
                "source": {
                    "params": pfl_src_path,
                    "schema": "leech",
                    "table_name": "pinkfloyd",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
                "destination": {
                    "params": pfl_dst_path,
                    "schema": "leech",
                    "table_name": "pinkfloyd",
                    "callbacks": "lib/.libs/leech_csv.so",
                },
            },
        },
    }
    with open(leech_conf_path, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Created leech config '{leech_conf_path}' with content:")
    with open(leech_conf_path, "r") as f:
        print(f.read())

    ##########################################################################
    # Create tables and commit
    ##########################################################################

    table = [
        ["first_name", "last_name", "born"],
        ["Paul", "McCartney", "1942"],
        ["Ringo", "Starr", "1940"],
        ["John", "Lennon", "1940"],
        ["George", "Harrison", "1943"],
    ]
    with open(btl_src_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(table)
    print(f"Created table '{btl_src_path}' with content:")
    with open(btl_src_path, "r") as f:
        print(f.read())

    table = [
        ["first_name", "last_name", "born"],
        ["Nick", "Mason", "1944"],
        ["Roger", "Waters", "1943"],
        ["Richard", "Wright", "1943"],
        ["Syd", "Barret", "1946"],
        ["David", "Gilmour", "1946"],
    ]
    with open(pfl_src_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(table)
    print(f"Created table '{pfl_src_path}' with content:")
    with open(pfl_src_path, "r") as f:
        print(f.read())

    command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
    assert execute(command, True) == 0

    ##########################################################################
    # Create delta patch file
    ##########################################################################

    lastknown = "0000000000000000000000000000000000000000"
    patchfile = os.path.join(tmp_path, "patchfile")
    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "diff",
        f"--block={lastknown}",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Apply delta patch file
    ##########################################################################

    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "patch",
        "--field=host_id",
        "--value=SHA=123",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Temporarily remove pinkfloyd from config
    ##########################################################################

    pfl = config["tables"].pop("PFL")
    with open(leech_conf_path, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Created leech config '{leech_conf_path}' with content:")
    with open(leech_conf_path, "r") as f:
        print(f.read())

    ##########################################################################
    # Modify table (with one insert, delete and update) followed by a commit
    ##########################################################################

    table = [
        ["first_name", "last_name", "born"],
        ["Paul", "McCartney", "1943"],
        ["John", "Lennon", "1940"],
        ["George", "Harrison", "1943"],
        ["Janis", "Joplin", "1943"],
    ]
    with open(btl_src_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(table)
    print(f"Modified table '{btl_src_path}' with content:")
    with open(btl_src_path, "r") as f:
        print(f.read())

    table = [
        ["first_name", "last_name", "born"],
        ["Nick", "Mason", "1944"],
        ["Roger", "Waters", "1944"],
        ["Richard", "Wright", "1942"],
        ["Syd", "Barret", "1946"],
        ["Eric Patrick", "Clapton", "1945"],
    ]
    with open(pfl_src_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(table)
    print(f"Modified table '{pfl_src_path}' with content:")
    with open(pfl_src_path, "r") as f:
        print(f.read())

    command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
    assert execute(command, True) == 0

    ##########################################################################
    # Create delta patch file
    ##########################################################################

    with open(os.path.join(tmp_path, "SHA=123"), "r") as f:
        lastknown = f.read().strip()
    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "diff",
        f"--block={lastknown}",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Apply delta patch file
    ##########################################################################

    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "patch",
        "--field=host_id",
        "--value=SHA=123",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Put pinkfloyd back into config
    ##########################################################################

    config["tables"]["PFL"] = pfl
    with open(leech_conf_path, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Created leech config '{leech_conf_path}' with content:")
    with open(leech_conf_path, "r") as f:
        print(f.read())

    ##########################################################################
    # Modify table (with one insert, delete and update) followed by a commit
    ##########################################################################

    table = [
        ["first_name", "last_name", "born"],
        ["James Marshall", "Hendrix", "1942"],
        ["John", "Lennon", "1941"],
        ["George", "Harrison", "1943"],
        ["Janis", "Joplin", "1943"],
    ]
    with open(btl_src_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(table)
    print(f"Modified table '{btl_src_path}' with content:")
    with open(btl_src_path, "r") as f:
        print(f.read())

    table = [
        ["first_name", "last_name", "born"],
        ["Nick", "Mason", "1944"],
        ["Kieth", "Richards", "1943"],
        ["Richard", "Wright", "1943"],
        ["Syd", "Barret", "1946"],
        ["Eric Patrick", "Clapton", "1945"],
    ]
    with open(pfl_src_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(table)
    print(f"Modified table '{pfl_src_path}' with content:")
    with open(pfl_src_path, "r") as f:
        print(f.read())

    command = [bin_path, "--debug", f"--workdir={tmp_path}", "commit"]
    assert execute(command, True) == 0

    ##########################################################################
    # Create delta patch file
    ##########################################################################

    with open(os.path.join(tmp_path, "SHA=123"), "r") as f:
        lastknown = f.read().strip()
    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "diff",
        f"--block={lastknown}",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0

    ##########################################################################
    # Apply delta patch file
    ##########################################################################

    command = [
        bin_path,
        "--debug",
        f"--workdir={tmp_path}",
        "patch",
        "--field=host_id",
        "--value=SHA=123",
        f"--file={patchfile}",
    ]
    assert execute(command, True) == 0
