# AZORK

A GPG capable WAV audio steganography tool for Linux. This is a physics project aligned with the cybersecurity curriculum of the university. The idea is to study what happens when we change the least significant bit of an audio sample, and whether that change is actually heard by a person.

This project was inspired by [wav-creator](https://github.com/dream-wa1ker/wav-creator.git), and is built on top of the WAV-generation idea from that project. The wav-creator code helped me understand how samples are generated and written into a WAV file. AZORK takes that idea further: instead of generating a new sine wave, it reads an existing WAV and stores a payload in its sample data.

> [!NOTE]
> I am not going to explain the whole WAV file type, RIFF header, and all its structures here. If you want to know how a basic WAV file is created, then you can check [wav-creator](https://github.com/dream-wa1ker/wav-creator.git). It has lots of comments; if you remove all those comments, it will be about 100 lines of code.

## Table of Contents

- [What this project does](#what-this-project-does)
- [The actual idea: changing the last bit](#the-actual-idea-changing-the-last-bit)
- [A little about digital audio](#a-little-about-digital-audio)
- [What does a person hear?](#what-does-a-person-hear)
- [Why WAV?](#why-wav)
- [The WAV header: wav-creator vs AZORK](#the-wav-header-wav-creator-vs-azork)
- [How AZORK stores and retrieves the payload](#how-azork-stores-and-retrieves-the-payload)
- [Why is the audio file so big?](#why-is-the-audio-file-so-big)
- [Will the size or hidden data cause suspicion?](#will-the-size-or-hidden-data-cause-suspicion)
- [Where else can this idea be used?](#where-else-can-this-idea-be-used)
- [A lab experiment you can actually do](#a-lab-experiment-you-can-actually-do)
- [Build and run](#build-and-run)
- [Limitations](#limitations)
- [References](#references)

## What this project does

AZORK hides a file inside a WAV audio file. It does this by replacing the least significant bit (LSB) of audio samples with bits from the payload. Later, the program can read those same sample positions and recover the file.

There are two modes:

- **Raw mode:** the payload file is stored directly.
- **GPG mode:** GPG signs and encrypts the payload first, and AZORK embeds the resulting OpenPGP data.

So, what is the point of hiding it in audio if we can just encrypt the file? Encryption and steganography answer different questions. Encryption is about making the contents unreadable. Steganography is about trying to make the existence of the contents less obvious. AZORK's GPG option protects the payload, but it does not make the LSB pattern invisible.

This is a prototype. The main point is to understand the idea, study the audio changes, and see the difference between hearing a change and detecting a change by analysis.

## The actual idea: changing the last bit

Let us look at a sample as an integer. For the WAV format AZORK expects, each channel sample is a signed 32-bit integer. In binary, the rightmost bit is the least significant bit. It has the smallest place value: one.

For example, if a sample ends in `...0` and the hidden bit is `1`, changing that last bit adds one to the integer. If it ends in `...1` and the hidden bit is `0`, changing that last bit subtracts one. If the sample already has the bit we need, it does not change at all.

The program's operation is basically this:

```text
new_sample = (old_sample AND ...11111110) OR hidden_bit
```

That means the sample changes by at most one integer step. For a signed 32-bit sample, one step is extremely small compared with the full-scale range. As a rough comparison, one step is about 1 / 2^31 of full-scale amplitude, or around -186.6 dB relative to full scale.

But here is the important part: **small does not mean zero**. The sample really changes. A computer can compare the old and new sample values and see the difference. Whether a human can hear the difference is a separate question.

The LSB is not some magical bit that can never affect sound. It is just the bit with the smallest numerical weight. If the recording is near silence, a one-step change is a larger percentage of the current sample than it would be near a loud peak. If many samples change, those individual changes form a small error signal across the audio.

So the accurate way to say it is: AZORK changes some samples by at most one code step. That change is tiny relative to full scale and may be masked by other sounds, but it is still measurable. Whether it can be heard depends on the recording and the listening conditions.

## A little about digital audio

A microphone captures changing air pressure and turns it into an electrical signal. An analogue-to-digital converter measures that signal at regular time intervals. Each measurement is a **sample**.

A simplified view is:

```text
x[n] = Q{x(n / fs)}
```

- `x(t)` is the original continuous signal.
- `fs` is the sample rate, in samples per second.
- `n` is the sample number.
- `Q{}` means that the measurement is rounded to one of the digital values available at the chosen bit depth.

The resulting WAV samples are not literally pressure values in pascals. They are numbers proportional to the audio signal, with the scaling determined by the recording equipment and format.

### Sample rate

The sample rate tells us how many samples are recorded per second **for each channel**. If the file is stereo at 96,000 Hz, the left channel has 96,000 samples each second and the right channel has another 96,000 samples. So altogether, there are 192,000 individual channel samples per second.

The sample rate also limits the frequencies that can be represented. The Nyquist-Shannon sampling theorem says that a band-limited signal needs a sampling rate greater than twice its highest frequency. In practice, audio hardware uses filters to reduce frequencies that would cause aliasing.

AZORK's default is 96 kHz, but this is a restriction chosen by the program. The LSB method itself does not require 96 kHz. The sine tone in the wav-creator example is 10 kHz, which is below the 22.05 kHz Nyquist frequency when using a 44.1 kHz sample rate.

### Bit depth and quantization

Bit depth says how many bits are used for each sample. An ideal signed 32-bit integer can represent values from -2,147,483,648 to +2,147,483,647. Each adjacent integer value is one quantization step apart.

When a continuous voltage is converted into a digital integer, it has to be rounded to one of the available levels. The difference between the real value and the stored value is called **quantization error**. Actual recording equipment also has microphone noise, electronic noise, and converter limitations, so bit depth alone does not tell us the complete noise level of a real recording.

## What does a person hear?

We use words like loudness, pitch, timbre, vibrato, tremolo, attack, and decay to describe different parts of what we hear. These are useful ideas, but they do not map one-to-one onto a single sample bit.

| What we talk about | What it mostly relates to | What the LSB change does |
|---|---|---|
| **Loudness** | Perceived strength of sound. It relates to sound pressure, level, frequency content, duration, and context. | AZORK does not turn the volume up or down. It adds tiny sample errors, which can slightly change measured peak or RMS level. |
| **Pitch** | The frequency or periodicity we hear as high or low. | AZORK does not intentionally change the main frequency. The small errors may add very low-level extra spectral content. |
| **Timbre** | The quality that makes two sounds with similar pitch and loudness sound different. It relates to the spectrum, transients, and changes over time. | Any waveform change can technically affect the spectrum. Whether that difference changes perceived timbre is a listening question. |
| **Vibrato** | A variation in pitch over time. | AZORK does not intentionally vary pitch. It changes sample values by tiny amounts. |
| **Tremolo** | A variation in loudness or amplitude over time. | AZORK does not intentionally vary amplitude. It may slightly affect the samples that form the amplitude envelope. |
| **Attack** | How a sound begins and rises. | The method does not reshape the attack, but errors in a quiet onset can be relatively more noticeable. |
| **Decay / release** | How a sound fades after the onset or after the source stops. | The method does not intentionally change the fade, but quiet tails have less surrounding sound to mask small errors. |

Vibrato and tremolo are easy to mix up. Vibrato is a change in frequency or pitch; tremolo is a change in amplitude or loudness.

### What about masking?

A loud or complex sound can make a quieter sound harder to hear. This is called **auditory masking**. It can happen when sounds occur together, or when one sound happens shortly before or after another. Audio compression systems use models of hearing to decide where quantization noise is less noticeable.

This helps explain why a small error may be harder to notice in some speech or music. A real recording can include voices, breaths, instruments, room reflections, microphone noise, and background sounds. The signal is changing naturally, which can make a tiny extra change harder to pick out by casual listening.

But natural audio does not guarantee that the hidden changes cannot be heard. Quiet passages, fades, and pauses can have less masking. A clean sine wave is useful for a controlled demonstration, while speech and music are useful for seeing what happens with more realistic recordings. Also, a computer detector does not need to hear the change as a person does. It can look for patterns in the sample values.

If we want to claim that people cannot hear a difference, we need to test that properly. We should compare level-matched original and embedded files, randomize which one is played, include a hidden reference, use multiple listeners and trials, and write down the listening conditions. Just listening once and saying “I cannot hear it” is a useful observation, but it is not a controlled result. ITU-R BS.1116 describes a method for assessing small audio impairments.

## Why WAV?

WAV is useful for this project because it commonly stores uncompressed PCM audio. The samples are available as direct numbers, and AZORK can edit their bits without first decoding a complicated lossy representation.

WAV is a **container**, not a promise that the audio is always PCM or always uncompressed. A WAV file can contain different audio encodings. AZORK only supports a narrow type of WAV: stereo, 32-bit PCM, 8 bytes per stereo frame, and a sample rate matching the selected value.

The other reason for using uncompressed PCM is that the sample values stay directly accessible. A lossy format such as MP3 transforms and discards audio information. If an embedded WAV is converted to MP3 and then back to WAV, the samples will not be the same, and the hidden LSB data will likely be damaged. Even resampling, changing bit depth, adjusting gain, normalizing, or applying dither can change sample values and break extraction.

WAV is good for a clear lab experiment. It is not automatically the best choice for small file size, and the format by itself does not make hidden data secure.

## The WAV header: wav-creator vs AZORK

I said above that I am not going to explain the entire WAV format, but we need a small amount of it to understand what both programs are doing.

The sample bytes alone do not tell a player what they mean. The file needs information such as the channel count, sample rate, bit depth, and where the audio data begins. WAV stores this information in RIFF chunks. A basic PCM WAV often has a 44-byte layout, but 44 bytes is only the common simple case. Other chunks can be present, so the audio data can begin later.

A common basic PCM layout looks like this:

| Offset | Size | Field | Why it is there |
|---:|---:|---|---|
| 0 | 4 | `RIFF` | Says that this is a RIFF file |
| 4 | 4 | RIFF size | Says how many bytes follow in the RIFF chunk |
| 8 | 4 | `WAVE` | Says that this RIFF file contains WAVE data |
| 12 | 4 | `fmt ` | Starts the format chunk |
| 16 | 4 | Format chunk size | Usually 16 for basic PCM format information |
| 20 | 2 | Audio format code | `1` is ordinary integer PCM |
| 22 | 2 | Channel count | For example, 1 for mono or 2 for stereo |
| 24 | 4 | Sample rate | Samples per second for each channel |
| 28 | 4 | Byte rate | Bytes of sample data per second |
| 32 | 2 | Block alignment | Bytes in one sample frame across all channels |
| 34 | 2 | Bits per sample | The bit depth for each channel sample |
| 36 | 4 | `data` | Starts the audio data chunk |
| 40 | 4 | Data size | Number of audio data bytes after this field |
| 44 | varies | Sample bytes | The actual audio values |

For PCM, `block_align` is the channel count multiplied by the bytes per sample. The byte rate is the sample rate multiplied by block alignment. Stereo sample data is usually interleaved: left sample, right sample, next left sample, next right sample.

Now the comparison. The wav-creator writes a new basic WAV header. AZORK does not write a new WAV header at all. It reads the existing WAV, finds the `data` chunk, changes some bytes in that chunk, and writes the original buffer back out.

| Topic | wav-creator | AZORK |
|---|---|---|
| Main purpose | Generate a WAV file containing a sine wave | Hide or recover a payload in an existing WAV file |
| Header handling | Defines a `WAVHeader` struct and writes it to the file | Does not define or write a serialized WAV header struct |
| What its struct represents | A fixed, basic 44-byte PCM header | AZORK's `WavInfo` is only an in-memory summary of parsed values and offsets |
| Audio format | Mono, 16-bit integer PCM | Stereo, 32-bit integer PCM |
| Sample rate in the pasted C | 44,100 Hz | 96,000 Hz by default; can be changed with `--sample-rate` |
| Where audio begins | Assumes samples start directly after its 44-byte header | Scans chunks to find the `data` chunk; it can be after other chunks |
| What happens to sample data | Creates new `int16_t` samples from a sine equation | Uses the existing samples and changes their LSBs |
| Byte order | Writes integer fields and samples from native memory | Uses explicit little-endian helpers for RIFF integer fields |
| Main lesson | How to generate samples and construct a simple WAV | How to alter sample LSBs and extract a bitstream |

There are a couple of details worth knowing when comparing the files:

- The wav-creator code says in one comment that the `fmt ` subchunk is 24 bytes. The format body is 16 bytes for basic PCM. The whole `fmt ` chunk is 24 bytes when the 8-byte tag and size are included.
- `__attribute__((packed))` controls padding between struct fields. It does not convert integers to little-endian. The wav-creator's direct struct write works as intended on common little-endian Linux machines, but a portable writer should serialize the fields explicitly.
- The pasted wav-creator C says `DURATION = 60`, while its walkthrough shows a one-second example. The actual C constant is the source for the actual file duration.
- AZORK's `WavInfo` is not a WAV header. It is a place in memory to keep information that the parser reads from the file. The original WAV bytes, including chunks before the audio, stay in the buffer.

AZORK checks the RIFF/WAVE identifiers, finds `fmt ` and `data`, and validates some audio properties. It does not check every possible WAV consistency field, and it does not support every WAV encoding. So, having a `.wav` extension is not enough to guarantee the file will work.

## How AZORK stores and retrieves the payload

AZORK reserves the first samples in the audio data for its own header and payload length. After that comes the payload itself.

| Part | Bytes | Samples used | Purpose |
|---|---:|---:|---|
| AZORK metadata | 100 | 800 | Magic `AZRK`, version, flags, type, and reserved bytes |
| Payload length | 4 | 32 | Says how many payload bytes follow |
| Payload | `N` | `8 × N` | The hidden file bytes, one bit per sample |

The bits of each byte are stored most-significant-bit first. The length field is stored as a 32-bit little-endian value.

### Encoding

1. The command-line arguments are parsed.
2. The full WAV file is read into memory.
3. The program finds the `fmt ` and `data` chunks and checks the WAV properties it supports.
4. It calculates the available capacity.
5. It reads the payload. In GPG mode, GPG signs and encrypts it first.
6. It writes the AZORK header, the payload length, and the payload into the sample LSBs.
7. It writes the modified WAV buffer to the output file.

### Decoding

1. The WAV is read and checked.
2. AZORK extracts the first 100 metadata bytes from the sample LSBs.
3. It checks the magic, version, and payload type.
4. It extracts the payload length and checks that the payload fits inside the audio data.
5. It extracts the payload.
6. Raw data is written as-is. OpenPGP data is passed to GPG to decrypt and verify.

The header is not a checksum or an authentication code. Raw mode has no protection against accidental sample changes. GPG mode adds cryptographic protection for the payload, assuming the keys and signatures are handled correctly.

## Why is the audio file so big?

Because uncompressed PCM stores every sample directly. The rough formula for the audio data size is:

```text
duration × sample rate × channels × bits per sample / 8
```

For AZORK's default format:

```text
1 second × 96,000 samples/second/channel × 2 channels × 32 bits/sample / 8
= 768,000 bytes per second
```

That is 768 kB per second, or about 46.08 MB per minute, plus the WAV headers and any extra chunks. For comparison, 16-bit stereo at 44.1 kHz uses about 176,400 bytes per second. AZORK's default audio data rate is about 4.35 times larger because it uses both a higher sample rate and double the bit depth.

The wav-creator C file uses 16-bit mono at 44.1 kHz and declares a 60-second duration. That is 2,646,000 samples and 5,292,000 bytes of sample data, plus its 44-byte header. If the duration were one second, it would be 88,200 bytes of sample data.

WAV files are big because they keep the PCM samples directly. That is useful for this experiment because we can access the exact sample values, but it costs storage. Compression can make files smaller; lossy compression changes the audio samples, and AZORK does not support compressed audio formats.

## Will the size or hidden data cause suspicion?

The embedding does not make the WAV bigger. AZORK replaces bits in samples that are already there, so the output WAV has the same file size as the input WAV.

But, yes, the size or format can still attract attention. An uncompressed 96 kHz, 32-bit stereo WAV can be much larger than an ordinary compressed music file. Someone may wonder why that format was chosen. Also, the hidden bits are placed sequentially in one predictable sample bit, which can be inspected by analysis. The AZORK header itself is also a marker if someone knows to look for it.

So it would not be accurate to say the data cannot cause suspicion. The carrier does not grow due to embedding, but the carrier's size and the embedding pattern can still be noticed. GPG can protect the contents, but it does not hide that a pattern is present.

At the default rate, AZORK visits 96,000 samples per second in each of two channels. That is 192,000 bit positions each second. Dividing by eight gives 24,000 bytes per second in theory. AZORK reserves 832 samples for its metadata and length, so the actual capacity is about 23,896 bytes per second of audio, before GPG overhead. The GPG output is larger than the original file, so it uses more capacity.

The capacity formula in the code is:

```text
(data chunk bytes / 4 - 832) / 8 bytes
```

This is the capacity of the current method, not a promise that embedding that much will be undetectable. This project is useful for studying that trade-off: large uncompressed carriers are easy to edit sample-by-sample, but size and statistical patterns can be clues.

## Where else can this idea be used?

The wider concepts are useful in a few areas:

- **Cybersecurity classes:** compare encryption, steganography, and steganalysis.
- **Digital signal processing:** measure quantization, sample error, spectra, RMS, and signal-to-noise ratio.
- **Audio perception research:** compare what a graph or detector finds with what listeners report.
- **Watermarking research:** study how information can be associated with media. Watermarking has different goals and is not the same as this simple sequential LSB method.
- **Media forensics:** study how edits and transformations affect sample data and whether traces can be found.
- **File-format learning:** understand why a file needs information about how its bytes should be interpreted.

These are areas where the ideas can be studied. AZORK itself is not a robust watermarking or production steganography system.

This project is not complete and there are more features that could be added. Support for other audio formats, including lossy formats, could be explored later. But that needs proper know-how about how those formats store audio, their headers or frames, and how the audio is transformed. MP3 is not just a WAV file with a different header. A lossy codec changes the audio representation, so the current LSB method cannot simply be carried across and expected to work.

## A lab experiment you can actually do

To make the project a physics experiment and not only a code demonstration, separate the measurements from the listening test:

1. Take a supported WAV file and keep an unchanged copy.
2. Embed a small payload and decode it again. Confirm that the recovered bytes match.
3. Compare every original sample with its embedded version. Count how many samples changed and check that no changed sample differs by more than one code step.
4. Calculate the difference signal:

   ```text
   e[n] = embedded[n] - original[n]
   ```

5. Measure the peak and RMS of that difference. State exactly how you calculated any signal-to-noise ratio.
6. Plot a short part of the original waveform and plot the difference separately. The difference might be too small to see on the original waveform's vertical scale.
7. Listen to original and embedded audio at matched playback levels. Randomize playback order and, if possible, do not tell listeners which is which.
8. Test different material: a sine tone, speech, music, and quiet passages. Record the playback device, level, room, number of listeners, and number of trials.
9. Try resampling, changing bit depth, adjusting gain, and lossy export. Check whether extraction still works.
10. Discuss whether a computer can find the LSB pattern even when listeners report no audible difference.

That gives us three different results to talk about: **the bytes changed; the measurements may detect the change; and listeners may or may not hear it.** They should not be treated as the same claim.

## Build and run

### Requirements

- Linux or another POSIX-like environment
- GCC with C23 support
- GNU Make
- GPG, if using GPG mode

### Build

```sh
make
```

The executable is created at `build/bin/azork`.

```sh
make build  # Build the executable
make run    # Build and print the help message
make clean  # Remove the build directory
make test   # Build, print help, then clean; this is not an encode/decode test suite
```

For Arch Linux, the project includes a `PKGBUILD`:

```sh
makepkg -si
```

### Hide a raw file

```sh
build/bin/azork \
  --encode secret.txt \
  --from music.wav \
  --to music-with-payload.wav
```

Short options are available too:

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

Give at least one GPG recipient. GPG needs the recipient's public key. It also needs a signing key; use `--signer` to choose one explicitly.

```sh
build/bin/azork \
  --encode secret.txt \
  --from music.wav \
  --to music-with-payload.wav \
  --gpg \
  --recipient recipient@example.com \
  --signer sender@example.com
```

You can repeat `--recipient` for multiple recipients:

```sh
--recipient alice@example.com --recipient bob@example.com
```

### Recover a GPG payload

The recipient needs the matching private key. GPG may request a passphrase depending on the local key setup.

```sh
build/bin/azork \
  --decode music-with-payload.wav \
  --to recovered.txt \
  --gpg
```

### Use a different sample rate

The expected sample rate must match the WAV during both encoding and decoding. For a 48 kHz WAV:

```sh
build/bin/azork -e secret.txt -f music.wav -t hidden.wav --sample-rate 48000
build/bin/azork -d hidden.wav -t recovered.txt --sample-rate 48000
```

Show all options:

```sh
build/bin/azork --help
```

## Limitations

- AZORK supports a narrow selection of WAV properties. A `.wav` extension does not guarantee compatibility.
- The method embeds data sequentially in one predictable bit position, so it is simple to demonstrate but not designed to resist steganalysis.
- Editing, resampling, converting bit depth, adjusting gain, normalizing, dithering, or lossy compression may damage or erase the payload.
- Raw mode does not encrypt, authenticate, or add error correction to the payload.
- GPG mode encrypts and signs the payload, but does not hide the fact that sample bits have been used.
- The program reads complete files into memory, so memory use grows with the WAV and payload sizes.
- Output files are opened for writing and can overwrite an existing file at that path.
- The Makefile's `test` target builds the program, prints help, and cleans. It is not an automated correctness test.

## References

- [Microsoft Learn: Resource Interchange File Format (RIFF)](https://learn.microsoft.com/en-us/windows/win32/xaudio2/resource-interchange-file-format--riff-)
- [McGill University: WAVE file specifications](https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html)
- [ITU-R Recommendation BS.1116: Methods for the subjective assessment of small impairments in audio systems](https://www.itu.int/rec/R-REC-BS.1116-3-201502-I/en)
- [Research example discussing direct LSB substitution in audio](https://www.sciencedirect.com/science/article/pii/S2214212615000034)

