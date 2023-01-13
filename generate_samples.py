def generate_samples():
    with open("beatles.csv", "w") as f:
        f.write(
            "firstname,lastname,role,born\r\n"
            + 'Paul,McCartney,"piano, vocals",1942\r\n'
            + 'Ringo,Starr,"drums, vocals",1940\r\n'
            + 'John,Lennon,"vocals, lead guitar",1940\r\n'
            + 'George,Harrison,"vocals, rythm guitar",1943'
        )

    with open("pinkfloyd.csv", "w") as f:
        f.write(
            "id,firstname,lastname,role\r\n"
            '0,Syd,Barret,"vocals, guitar"\r\n'
            "1,Nick,Mason,drums\r\n"
            '2,Roger,Waters,"vocals, bass"\r\n'
            '3,Richard,Wright,"vocals, keyboard"\r\n'
            '4,David,Gilmour,"vocals, guitar"\r\n'
        )


if __name__ == "__main__":
    generate_samples()
