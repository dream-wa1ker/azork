# AZORK

AZORK is a Linux command-line tool for acoustic steganography. It hides a file inside a WAV audio file and can later recover it. The project was developed as a physics lab prototype for an acoustics and ultrasonics course.

AZORK also has an optional GPG mode. In that mode, it signs and encrypts the payload before embedding it in the audio; during recovery, GPG decrypts the payload and checks its signature.

## How it works

AZORK stores data in the least significant bit (LSB) of the first byte of each 32-bit PCM audio sample. Each payload bit uses one sample. The program changes at most one bit in a sample, which keeps the numerical change to the audio sample very small.

The hidden stream contains:

1. A 100-byte AZORK header, including a magic value, format version, and payload type.
2. A 4-byte payload length.
3. The payload bytes (raw file data or GPG output).

The header and payload are embedded in the WAV `data` chunk. Other chunks, such as audio metadata before the data chunk, are preserved as they were in the input file.

## WAV requirements

The input must be a RIFF/WAVE file with:

- Stereo audio (2 channels)
- 32-bit PCM samples
- 8-byte stereo frame alignment
- A sample rate of 96,000 Hz by default

Use `--sample-rate` to set a different expected sample rate for both encoding and decoding. The program supports standard PCM and WAVE_FORMAT_EXTENSIBLE files whose subformat is PCM. Other WAV encodings, including compressed audio and floating-point samples, are not supported.

## Capacity

AZORK uses one sample per embedded bit. It reserves 832 samples for the header and payload length, then uses another 8 samples per payload byte. With stereo audio, the data chunk has two samples per frame, so the approximate maximum payload capacity is:

```text
(data chunk bytes / 4 - 832) / 8 bytes
```

The actual payload must fit in the WAV's audio data. In GPG mode, the encrypted and signed output is larger than the original file, so it uses more capacity.

## Build

Requirements:

- Linux or another POSIX-like environment
- GCC with C23 support
- GNU Make
- GPG, if using GPG mode

From the project directory, build the executable with:

```sh
make
```

The executable is created at `build/bin/azork`.

Other Make targets:

```sh
make build  # Build the executable
make run    # Build and print the command-line help
make clean  # Remove the build directory
make test   # Build, print help, then clean; this is not an encode/decode test suite
```

For Arch Linux, the project includes a `PKGBUILD`; build and install it with:

```sh
makepkg -si
```

## Usage

### Hide a file (raw mode)

```sh
build/bin/azork \
  --encode secret.txt \
  --from music.wav \
  --to music-with-payload.wav
```

Short options are also available:

```sh
build/bin/azork -e secret.txt -f music.wav -t music-with-payload.wav
```

### Recover a raw file

```sh
build/bin/azork \
  --decode music-with-payload.wav \
  --to recovered.txt
```

### Hide a signed and encrypted file

Provide at least one GPG recipient. The recipient must have a public key available to the sender. GPG must also be able to select a signing key; `--signer` can specify one explicitly.

```sh
build/bin/azork \
  --encode secret.txt \
  --from music.wav \
  --to music-with-payload.wav \
  --gpg \
  --recipient recipient@example.com \
  --signer sender@example.com
```

Repeat `--recipient` to encrypt for more than one recipient:

```sh
--recipient alice@example.com --recipient bob@example.com
```

### Recover a GPG payload

The recipient needs the matching private key, and GPG may ask for its passphrase depending on the local GPG setup.

```sh
build/bin/azork \
  --decode music-with-payload.wav \
  --to recovered.txt \
  --gpg
```

### Use a different expected sample rate

The chosen rate must match the WAV file for both encoding and decoding. For example, for a 48 kHz WAV:

```sh
build/bin/azork -e secret.txt -f music.wav -t hidden.wav --sample-rate 48000
build/bin/azork -d hidden.wav -t recovered.txt --sample-rate 48000
```

### Help

```sh
build/bin/azork --help
```

## Limitations and handling notes

- AZORK modifies the audio sample data. Avoid processing the resulting WAV with tools that resample, normalize, convert formats, or recompress it; such changes can damage or erase the hidden bits.
- Raw mode does not encrypt or authenticate the payload. Anyone who knows the format can extract it, and accidental changes to the audio can corrupt it.
- GPG mode protects the payload cryptographically, but the existence of hidden data is not concealed from steganalysis. The embedding pattern is sequential and uses the same sample bit throughout.
- This is a prototype with narrow WAV-format support, not a general-purpose steganography or secure communications system.
- The tool reads the complete WAV and payload into memory, so memory use grows with file size.
- Check that the selected output path is safe to overwrite. Output files are created with truncation if they already exist.

## License

See [`LICENSE`](LICENSE) for the project license.

## More 

This is actually, a physics project aligned with the cybersecurity curriculum of University. So, the main part includes the physics of how this project works, why changing the LSB does nothing to audio quality, or maybe, if it does, the impact is less enough and not perceivable by people. Music can be found, but the finding of the changes in the audio recorded, and the speech of the person is harder due to a more natural production environment that the studio quality music. To see about the physics of this, start [here](docs/physics/intro.md);

>[!NOTE]
> I am not going to explain the format of the WAV file type and what RIFF header, it's structures look like, if you want to know about it too, then you can check this [wav-creator](https://github.com/dream-wa1ker/wav-creator.git). Made with lots of comments, if you remove all those comments, will be just a 100 lines of code...


