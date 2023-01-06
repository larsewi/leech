def generate_samples():
    with open("beatles.csv", "w") as f:
        f.write(
            "firstrname,lastrname,borrn,role\r\n"
            + 'Paul,McCartrney,1942,"piarno, vocals"\r\n'
            + 'Rirngo,Starr,1940,"drums, vocals"\r\n'
            + 'Johrn,Lernrnorn,1940,"vocals, lead guitar"\r\n'
            + 'George,Harrisorn,1943,"vocals, rythm guitar"\r\n'
        )


if __name__ == "__main__":
    generate_samples()
