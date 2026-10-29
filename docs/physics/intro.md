# AZORK: acoustic steganography as a physics and cybersecurity project

AZORK is a Linux command-line prototype that hides a file inside the sample data of a WAV audio file. It was developed as a physics project aligned with a university cybersecurity curriculum. The project brings together digital sampling, quantization, hearing and perception, file formats, and the difference between encryption and steganography.

The central question is not simply “does changing one bit make a sound?” It is: **what numerical change does an LSB substitution make to a sampled pressure waveform, how does that error relate to auditory perception, and what can an observer infer from the resulting file?**

This README explains the idea from the ground up, then connects the explanation to AZORK’s implementation. It is written so a student can present the project to a junior student and defend its assumptions to a professor.

> **Scope note:** AZORK reads enough of the WAV container to find and validate its PCM data. This write-up does not teach the complete RIFF/WAV specification. For a commented example of constructing a basic WAV file, see [wav-creator](https://github.com/dream-wa1ker/wav-creator.git).

## 1. The idea in one minute

A microphone converts changing air pressure into an electrical signal. A digital recorder measures that signal many times per second and stores each measurement as an integer called a **sample**. In AZORK’s supported WAV files, each channel sample is a signed 32-bit integer.

The last bit of a binary integer has the smallest place value. Replacing that bit changes a sample by at most one integer count. AZORK uses that bit as a tiny carrier for one hidden data bit. It writes a header and payload length first, then the payload, one bit per sample.

The numerical perturbation is very small relative to the full range of a 32-bit sample. That is a reason to expect it to be hard to hear under many conditions, but it is not proof of inaudibility. A listener, measurement system, or statistical detector can inspect the result in ways that are different from ordinary listening. The exact audio content, playback level, sample format, and listening conditions all matter.

Encryption and hiding solve different problems:

- **Encryption** aims to make the payload unreadable without the key.
- **Steganography** aims to make the presence of a payload less obvious.
- AZORK’s optional GPG mode encrypts and signs the payload before embedding it. It does not make the embedding pattern invisible.

## 2. From sound to stored samples

### 2.1 Sound pressure and the waveform

Sound in air is a traveling pressure variation. At a fixed microphone location, the pressure rises and falls over time. A microphone and recording circuit turn that variation into a voltage. An analogue-to-digital converter (ADC) measures the voltage at regular time intervals and maps each measurement to a digital number.

A simplified model is:

```text
x[n] = Q{x(n / fs)}
```

where:

- `x(t)` is the continuous signal at time `t`;
- `fs` is the sampling rate in samples per second;
- `n` is an integer sample index;
- `Q{}` is quantization, the conversion to one of a finite set of digital values.

The stored sequence is not literally a list of air-pressure values in pascals. It is a sequence of digital values proportional to the electrical signal, with a scale determined by the recording chain and file format.

### 2.2 Sampling rate

The sample rate says how many amplitude measurements are stored per second **for each channel**. At 96,000 Hz, the left channel has 96,000 samples each second and the right channel has another 96,000. A stereo file therefore contains 192,000 individual channel samples per second.

Sampling rate controls the highest frequency that can be represented without aliasing. For a band-limited signal, the Nyquist-Shannon criterion requires a sampling rate greater than twice the highest frequency of interest. Real ADC systems also use anti-aliasing filters. AZORK’s 96 kHz default is a program restriction, not a requirement of the LSB technique. The 10 kHz test tone in the pasted generator is below the 22.05 kHz Nyquist frequency of its 44.1 kHz sample rate.

### 2.3 Bit depth and quantization

Bit depth is the number of binary digits used for each sample. An ideal signed `N`-bit integer has `2^N` possible code values. Signed 32-bit PCM ranges from −2,147,483,648 through +2,147,483,647. The distance between adjacent integer values is one **least significant bit**, or one quantization step.

An ideal quantizer rounds a value to the nearest representable level. The difference between the original value and the quantized value is quantization error. In actual audio equipment, analogue noise, converter design, clocking, and the analogue signal chain also affect the noise floor; the nominal bit depth alone does not promise that every bit carries equally clean information.

## 3. Exactly what changing the LSB does

For a signed integer sample `s`, the least significant bit is the `2^0` place. AZORK’s operation is equivalent to:

```text
new_sample = (old_sample AND ...11111110) OR hidden_bit
```

The program clears the bottom bit, then writes either 0 or 1 into that position. If the old LSB already matches the hidden bit, the sample is unchanged. If it does not match, the integer changes by exactly +1 or −1, depending on the old value and bit transition. The largest absolute change is one code step.

For a 32-bit signed sample near full scale, one code step is approximately:

```text
1 / 2^31 ≈ 4.66 × 10^-10 of full-scale amplitude
```

That is about −186.6 dB relative to full scale for a single one-step amplitude difference under this simplified ratio. This figure is a scale comparison, **not** a guarantee that an embedded signal is inaudible: it does not account for how often samples change, the waveform, playback gain, the playback chain, or how a listener compares the signals.

If the hidden bit equals the original LSB, there is no change. If it differs, there is a one-code change. For payload bits that behave roughly randomly relative to the carrier LSB, around half of the visited samples will change and half may stay the same. Across a large number of modified samples, those small errors can be treated as an added error signal. One can measure it directly instead of relying only on a listening impression.

### 3.1 Why “one bit” is not the same as “no effect”

The digital sample values really do change. A waveform comparison can reveal this. The audio may still sound the same to a person because hearing has finite sensitivity and because the resulting error may be below the audible threshold in that context. These are different statements:

1. **Digital equality:** false; some stored sample values change.
2. **Measurability:** often true; a difference signal can expose the changed samples.
3. **Audibility:** depends on conditions and must be assessed perceptually.
4. **Statistical detectability:** depends on a detector and the carrier’s natural statistics; it does not require that the change be audible.

A robust claim for a lab report is therefore: “LSB substitution introduces a bounded one-code perturbation at each sample it visits. The perturbation is very small relative to full scale and may be perceptually masked under some conditions, but audibility and detectability are empirical questions.”

### 3.2 What happens near silence?

When a sample is near zero, a one-code perturbation is a larger fraction of that sample’s instantaneous amplitude than it is near full scale. A run of such modifications during digital silence can produce low-level structured or noise-like output. In ordinary real recordings, exact digital silence may be uncommon, but pauses, fades, and quiet passages still exist.

This is why “a one-step change is tiny” is not enough by itself to establish universal inaudibility. Psychoacoustic masking can make a low-level error less noticeable in the presence of other sound, but a quiet passage offers less masking. The effect should be tested with both active and quiet material.

## 4. How the change relates to hearing

Hearing does not decode a waveform by checking the LSB. The ear and brain respond to pressure variations after they have passed through the outer ear, middle ear, cochlea, auditory nerve, and higher processing. Common descriptions of an audio signal—loudness, pitch, timbre, vibrato, tremolo, attack, and decay—refer to different aspects of that signal and its perception.

| Perceptual quality | Main signal property | What LSB embedding normally does to it | Useful way to think about it |
|---|---|---|---|
| **Loudness** | Perceived strength; related to sound pressure level, signal RMS/peak, frequency content, duration, and listening context | It does not intentionally change the gain or scale. It adds a very small sample error, which can make a minute change to measured RMS or peak | Loudness is not just the integer amplitude; it is a perceptual response to the reproduced sound |
| **Pitch** | Perceived frequency of a periodic or near-periodic component | It does not intentionally shift the fundamental frequency. Tiny errors can add spectral energy, but normally do not move the tone’s main frequency | Pitch is mostly about periodicity/frequency, not the final bit of each sample |
| **Timbre** | The character that distinguishes sounds with similar pitch and loudness; related to spectrum, transients, and time variation | Any added error technically changes the waveform and can alter its spectrum slightly; whether this is audible depends on level and masking | Timbre is a broad quality, so there is no single “timbre number” that LSB changes |
| **Vibrato** | A periodic variation in pitch/frequency | The embedding does not deliberately modulate pitch. It may add a tiny error around the existing modulation | Vibrato is frequency modulation, not amplitude modulation |
| **Tremolo** | A periodic variation in amplitude/loudness | The embedding does not deliberately modulate amplitude. It may slightly perturb the samples that form the amplitude envelope | Tremolo is amplitude modulation; do not confuse it with vibrato |
| **Attack** | The onset and rise of a sound | A one-code change does not redesign the onset, but an error in a quiet onset can have a larger relative size | Transients and beginnings can be perceptually revealing |
| **Decay / release** | How sound falls away after its onset or after excitation stops | The method does not intentionally alter the decay envelope, but a low-level tail has less masking and is worth inspecting | The tail of a note or room reverberation can reveal low-level noise |

These are useful categories for discussion, not a promise that every listener will rate each one identically. Listening level, headphones/speakers, room noise, expectation, hearing ability, and repeated comparison all influence judgments.

### 4.1 Auditory masking and natural recordings

A stronger sound can make a nearby weaker sound less audible. This is called masking. It can happen across nearby frequencies (simultaneous masking) and over short time intervals (temporal masking). Audio codecs use psychoacoustic models to allocate precision in ways that exploit such limits.

A musical mix or field recording may mask low-level error differently from a clean test tone. Speech recordings can include changing vocal sounds, breaths, room reflections, microphone self-noise, and background activity. These variations make the signal less stationary and can make informal listening comparisons harder. However:

- Naturalness does **not** guarantee that a payload is inaudible.
- Studio music is not automatically easier to detect by ear; its masking, mastering, and spectral content vary.
- A simple steady tone is useful for controlled demonstrations, while real speech/music is useful for testing realistic material.
- Statistical steganalysis is a different question from whether an ordinary listener hears a difference.

To support an audibility claim scientifically, use a controlled listening test with level-matched original and embedded versions, randomized presentation, a hidden reference, enough listeners/trials, and documented conditions. ITU-R BS.1116 is an established recommendation for assessing small impairments in audio systems. A casual “I can’t hear it” observation is useful as a first demonstration, but it is not equivalent to a controlled result.

## 5. Why WAV is used here

WAV is a container format commonly used to hold uncompressed PCM audio. Its advantages for this experiment are practical:

- PCM samples are stored as explicit numbers that the program can inspect and edit.
- The format is widely supported by audio tools and operating systems.
- It avoids lossy compression changing the sample values during a normal save/decode cycle.
- A simple PCM WAV gives a direct path from sample index to stored bytes, which makes the algorithm teachable and measurable.

WAV is not inherently “higher quality” than every other possible format, nor is WAV itself a guarantee of uncompressed PCM. WAV can carry different encodings. AZORK specifically accepts a narrow subset: PCM with two channels, 32 bits per sample, 8 bytes per stereo frame, and the selected sample rate.

Lossless PCM is convenient for a controlled experiment because the embedding can be written and extracted from the same exact sample sequence. Lossy formats such as MP3 or AAC transform audio and discard information; converting an embedded WAV to one of those formats can alter or remove the LSB data. Even some ordinary audio-processing operations—resampling, gain changes, bit-depth conversion, normalization, or dithering—can change the values and break extraction.

## 6. WAV header and the two project files

### 6.1 Why a WAV header exists

The audio samples alone do not tell a player how to interpret them. A sequence of bytes could mean 16-bit mono at 44.1 kHz, 24-bit stereo at 48 kHz, or many other things. The WAV container stores identifying tags, lengths, and format properties so software can locate the audio and interpret the sample bytes.

WAV is based on RIFF, a chunked format. A RIFF/WAVE file begins with a top-level RIFF chunk. Inside it are chunks such as `fmt ` (format description) and `data` (audio bytes). Each chunk has an identifier, a size, and chunk data; optional chunks may be present. A basic PCM WAV often has a 44-byte layout, but **44 bytes is the common minimal layout, not a universal fixed header size**. Extra chunks make files longer and change where the `data` chunk begins.

A common 44-byte PCM layout is:

| File offset | Size | Field | What it tells the reader |
|---:|---:|---|---|
| 0 | 4 | `RIFF` | Identifies the RIFF container |
| 4 | 4 | RIFF chunk size | Bytes in the RIFF chunk after its first 8 bytes |
| 8 | 4 | `WAVE` | Identifies the RIFF form as WAVE |
| 12 | 4 | `fmt ` | Begins the format chunk |
| 16 | 4 | Format chunk size | Usually 16 for the basic PCM format body |
| 20 | 2 | Audio format code | `1` means integer PCM in the basic format |
| 22 | 2 | Channel count | For example, 1 mono or 2 stereo |
| 24 | 4 | Sample rate | Samples per second per channel |
| 28 | 4 | Byte rate | Bytes of sample data per second |
| 32 | 2 | Block alignment | Bytes in one complete frame across all channels |
| 34 | 2 | Bits per sample | Nominal bit depth of each channel sample |
| 36 | 4 | `data` | Begins the audio data chunk |
| 40 | 4 | Data chunk size | Number of following audio-data bytes |
| 44 | variable | PCM sample bytes | Interleaved channel samples in the declared encoding |

In stereo PCM, a frame contains one sample for the left channel and one for the right channel. Those channel samples are normally interleaved: left, right, left, right, and so on. `block_align = channels × bits_per_sample / 8`; `byte_rate = sample_rate × block_align` for PCM.

### 6.2 WAV-creator header versus AZORK’s WAV handling

The pasted wav-creator source defines and writes a 44-byte header struct for a specific generated file. AZORK does **not** define or write a replacement WAV header. It reads an existing RIFF/WAVE file, parses chunk locations into an in-memory `WavInfo` structure, changes bytes inside the existing `data` chunk, and writes the complete original byte buffer back out.

| Topic | Pasted wav-creator program | AZORK program |
|---|---|---|
| Main job | Generate a new simple WAV containing a synthesized sine wave | Hide or recover a payload inside an existing WAV’s sample data |
| Header representation | `WAVHeader` struct, marked packed; intended to be 44 bytes | No serialized WAV header struct; `WavInfo` holds parsed fields and offsets in memory |
| Does it write the WAV header? | Yes. It fills the struct fields and writes the struct bytes to the file | No. It preserves the input WAV bytes and writes them back after changing sample data |
| File structure assumption | Fixed, minimal 44-byte PCM header followed immediately by samples | Scans RIFF chunks to find `fmt ` and `data`; data may start after additional chunks |
| Audio format in the pasted C | Mono, 16-bit integer PCM | Requires stereo, 32-bit integer PCM; accepts standard PCM and extensible PCM subformat |
| Sample rate in the pasted C | 44,100 Hz | 96,000 Hz by default; `--sample-rate` changes the expected value |
| Sample content | Generates a mathematical sine wave | Uses whatever samples are already in the input WAV |
| Audio byte layout | Writes host-memory integer fields and `int16_t` sample array directly | Reads/writes individual file bytes and uses explicit little-endian helpers for WAV fields |
| Header size | The struct is the classic 44-byte layout | No fixed header-size assumption for where audio starts; uses chunk offsets |
| Main educational point | How sample values and a basic WAV container can be generated | How tiny sample-bit substitutions can carry a payload |

**Important portability detail:** WAV fields are little-endian. The wav-creator writes integer fields and 16-bit samples from native memory directly. That works as intended on common little-endian Linux machines, but a fully portable writer should explicitly serialize values in little-endian byte order instead of relying on host byte order. The `packed` attribute controls struct padding; it does not convert endianness. AZORK’s helper functions explicitly read and write little-endian values for RIFF fields.

The wav-creator comments describe the `fmt ` subchunk body as 24 bytes in one place; for the simple PCM layout it writes, the `fmt ` chunk’s declared body size is 16 bytes. Including its 8-byte chunk identifier and size field, the whole `fmt ` chunk occupies 24 bytes. That distinction is a common source of confusion.

### 6.3 What AZORK validates—and what it does not

AZORK validates the RIFF/WAVE identifiers, finds the first `fmt ` and `data` chunks, checks the format tag (or the PCM subformat GUID for extensible WAV), checks stereo, 32-bit samples, block alignment, sample rate, and data alignment. It preserves chunks by modifying only the `data` bytes in its original buffer.

The implementation is still a prototype parser. It does not validate every WAV consistency field (for example, the declared byte rate), and it does not fully validate all extensible-format details such as valid bits per sample. Its support should therefore be understood as “the narrow PCM files checked by this code,” not every file with a `.wav` extension.

## 7. How AZORK embeds and recovers data

### 7.1 Layout in the sample stream

The first sample used is the first 32-bit channel sample in the WAV `data` chunk. AZORK stores bits in sequence:

| Part | Bytes | Samples used | Purpose |
|---|---:|---:|---|
| AZORK metadata | 100 | 800 | Magic `AZRK`, version, flags, payload type, reserved bytes |
| Payload length | 4 | 32 | Payload length as a 32-bit little-endian integer |
| Payload | `N` | `8 × N` | Raw bytes or GPG output, most significant bit first within each byte |

The metadata marks the payload type as raw or OpenPGP. The length tells the decoder how many following bytes to extract. The remaining WAV chunks and samples after the embedded region are left as in the input.

### 7.2 Encoding path

1. Parse command-line options.
2. Read the whole WAV file into memory.
3. Find and validate the WAV format and data chunk.
4. Calculate the available capacity.
5. Read the payload, or use GPG to sign and encrypt it.
6. Create the AZORK metadata and encode the payload length.
7. Write the header, length, and payload bits into consecutive sample LSBs.
8. Write the modified full WAV byte buffer to the output path.

### 7.3 Decoding path

1. Read and validate the WAV file.
2. Extract the first 100 metadata bytes from sample LSBs.
3. Check the magic, version, and payload type.
4. Extract the 4-byte length and check it fits inside the data chunk.
5. Extract the declared payload.
6. Write raw bytes directly, or send OpenPGP data to GPG for decryption and signature verification.

The metadata is a format marker, not a cryptographic integrity check. In raw mode, accidental audio edits can corrupt data without AZORK detecting the corruption unless the header or bounds become invalid. GPG provides cryptographic protection for GPG-mode payloads, subject to the user’s key setup and verification of the signing key.

## 8. Why the WAV file is large

Uncompressed PCM stores every sample directly. A useful size formula is:

```text
audio data bytes = duration × sample rate × channel count × bits per sample / 8
```

For AZORK’s default format:

```text
1 second × 96,000 samples/s/channel × 2 channels × 32 bits/sample / 8
= 768,000 bytes per second
```

That is about 768 kB/s (decimal), or 46.08 MB per minute, before small container overhead. A 16-bit, 44.1 kHz stereo PCM file is about 176,400 bytes per second. AZORK’s default audio-data rate is about 4.35 times that common CD-audio PCM rate because it uses a higher sample rate and twice the bit depth.

The pasted sine-wave generator uses 16-bit mono at 44.1 kHz. Its actual C file declares a duration of 60 seconds, which produces 2,646,000 samples and 5,292,000 sample-data bytes (about 5.29 MB), plus the 44-byte simple header. The accompanying walkthrough text shows a one-second example in places; that is an inconsistency between the pasted code and its explanation. At one second, that configuration would use 88,200 sample bytes.

WAV is large because PCM prioritizes direct, uncompressed sample storage. Compression can reduce size, but lossy compression alters samples; lossless compression can reduce file size while preserving samples, but AZORK does not support compressed WAV encodings. Storage size and secrecy are separate issues.

## 9. Capacity and the “will the size look suspicious?” question

At 96 kHz stereo, AZORK processes 192,000 channel samples per second. It stores one payload bit per sample, so the raw theoretical embedding rate is:

```text
192,000 bits/s ÷ 8 = 24,000 bytes/s
```

After the fixed 832-sample header/length reservation, the capacity is approximately 23,896 bytes per second of audio, or about 23.3 KiB/s. The program’s exact capacity formula is:

```text
(max(data_chunk_bytes / 4 - 832, 0)) / 8 bytes
```

This is a theoretical maximum before GPG overhead. Signed-and-encrypted output is larger than the original payload, so it consumes more capacity.

Embedding does not increase the WAV file’s size: AZORK changes existing sample bits in place and writes the same-sized audio buffer. However, the carrier itself may be conspicuously large, and its format may be unusual for an ordinary recording. A file can also be inspected for LSB patterns, unusual sample statistics, or the AZORK header. Therefore, a large WAV may attract attention even if its payload makes no size difference. Conversely, a normal-sized file is not automatically undetectable.

The project should present this as a tradeoff and research question, not claim that it “avoids suspicion.” A high-quality uncompressed carrier offers stable, directly editable samples and capacity, but it can be much larger than compressed music and can preserve a detectable embedding pattern. Encryption protects payload contents; it does not erase these clues.

## 10. Where else the idea is useful

The broader ideas behind AZORK have legitimate uses in teaching, research, and media engineering:

- **Cybersecurity education:** demonstrate the distinction between encryption, steganography, and steganalysis.
- **Digital signal processing labs:** measure quantization steps, error signals, spectra, RMS levels, and signal-to-noise ratios.
- **Perceptual audio research:** compare objective signal differences with controlled listening results.
- **Watermarking research:** study how information can be associated with media, while noting that robust watermarking has different goals and often uses different domains/techniques.
- **Media provenance and forensics:** understand how sample-level edits may be detected or how transformations affect embedded information.
- **Protocol and file-format education:** learn why a container needs metadata describing how to interpret the bytes it carries.

These are broader application areas; AZORK itself is a simple sequential LSB prototype. It is not a robust watermarking system, a production steganography system, or a substitute for secure communication practices.

## 11. A practical experiment for the lab

A fair demonstration should separate **measurement**, **listening**, and **detection**:

1. Prepare a supported 32-bit stereo PCM WAV and preserve an untouched copy.
2. Embed a small payload and decode it to confirm exact recovery.
3. Compare original and stego samples. Count changed samples and verify that each changed sample differs by at most one code step.
4. Compute the difference signal `e[n] = stego[n] − original[n]`, its peak and RMS values, and an SNR measure. State the formula and normalization convention used.
5. Plot a short original waveform and the difference waveform separately. The difference may need a larger vertical scale to be visible.
6. Listen to level-matched original and stego versions using randomized, preferably blind comparisons. Include quiet passages, speech, music, and a controlled tone if available.
7. Record the equipment, playback level, listeners, trial count, and results. Avoid claiming “inaudible” from one person’s unblinded listening.
8. Test whether common transformations—resampling, bit-depth conversion, gain adjustment, or lossy export—break exact recovery.
9. Discuss whether a detector can identify the sequential LSB pattern even when listeners report no audible difference.

A particularly clear demonstration is to use one carrier with quiet sections and one with dense music. This makes it easier to discuss the difference between an absolute one-code perturbation, its relative size near silence, and perceptual masking in complex sound.

## 12. Usage

### Build

Requirements:

- Linux or another POSIX-like environment
- GCC with C23 support
- GNU Make
- GPG if using GPG mode

From the project directory:

```sh
make
```

The executable is created at `build/bin/azork`.

```sh
make build  # Build the executable
make run    # Build and print command-line help
make clean  # Remove the build directory
make test   # Build, print help, then clean; not a functional encode/decode test suite
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

Short options are available:

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

Specify at least one GPG recipient. GPG must have the recipient’s public key and be able to select a signing key. `--signer` can select the signing identity explicitly.

```sh
build/bin/azork \
  --encode secret.txt \
  --from music.wav \
  --to music-with-payload.wav \
  --gpg \
  --recipient recipient@example.com \
  --signer sender@example.com
```

Repeat `--recipient` for multiple recipients:

```sh
--recipient alice@example.com --recipient bob@example.com
```

### Recover a GPG payload

The recipient needs the matching private key. GPG may request a passphrase according to the local key setup.

```sh
build/bin/azork \
  --decode music-with-payload.wav \
  --to recovered.txt \
  --gpg
```

### Set a different expected sample rate

The rate must match the WAV during both encoding and decoding. For a 48 kHz file:

```sh
build/bin/azork -e secret.txt -f music.wav -t hidden.wav --sample-rate 48000
build/bin/azork -d hidden.wav -t recovered.txt --sample-rate 48000
```

Show all command-line options:

```sh
build/bin/azork --help
```

## 13. Limitations

- Only the WAV properties checked by the implementation are supported; a `.wav` extension alone is not enough.
- The embedding is sequential and uses one predictable bit position per sample. It is simple to explain, but not designed to resist steganalysis.
- Any audio-processing step that changes sample values may corrupt or erase hidden data.
- Raw mode has no encryption, authentication, or error correction.
- GPG mode encrypts and signs the payload but does not hide the fact that the WAV contains a structured bit pattern.
- The program reads complete files into memory, so memory use grows with file size.
- Output files are opened for writing and may overwrite an existing file with the same path.
- The Makefile’s `test` target is a build/help/cleanup sequence, not an automated correctness test.

## 14. Short presentation summary

> AZORK demonstrates acoustic steganography by replacing the least significant bit of 32-bit PCM audio samples with payload bits. Each changed sample differs by only one quantization step, which is extremely small relative to full scale and may be masked by other sound. But the change is real and measurable, and audibility depends on the signal and listening conditions. WAV is useful because its PCM samples are directly accessible and uncompressed, although that makes files large. The method is simple and educational, not guaranteed inaudible or resistant to detection. GPG can encrypt and sign the payload, while steganography addresses whether the payload is apparent in the first place.

## References

- [Microsoft Learn: Resource Interchange File Format (RIFF)](https://learn.microsoft.com/en-us/windows/win32/xaudio2/resource-interchange-file-format--riff-)
- [McGill University: WAVE file specifications and PCM format](https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html)
- [ITU-R Recommendation BS.1116: Methods for the subjective assessment of small impairments in audio systems](https://www.itu.int/rec/R-REC-BS.1116-3-201502-I/en)
- [Example research on limitations of direct audio LSB substitution](https://www.sciencedirect.com/science/article/pii/S2214212615000034)

