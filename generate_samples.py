def generate_samples():
    with open("beatles.csv", "w") as f:
        f.write(
            "firstname,lastname,role,born\r\n"
            + 'Paul,McCartney,"piano, vocals",1942\r\n'
            + 'Ringo,Starr,"drums, vocals",1940\r\n'
            + 'John,Lennon,"vocals, lead guitar",1940\r\n'
            + 'George,Harrison,"vocals, rythm guitar",1943'
        )


if __name__ == "__main__":
    generate_samples()
