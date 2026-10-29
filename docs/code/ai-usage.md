# AI Usage and Team Contributions

## Why this file is here

I want to be clear about how AI was used in this project. The short version is: **this is a vibe-documented, not vibe-coded project**—with some important details behind that sentence.

AZORK is my physics project, aligned with the cybersecurity curriculum of the university. The project was inspired by [wav-creator](https://github.com/dream-wa1ker/wav-creator.git), and the original code was built on top of its WAV-generation ideas and code. Four of my teammates also supported the project and made code changes.

## How AI was used

AI was used to refine and format parts of the code for a lab-assessment level of code quality. I gave the AI a specific constraint: keep the bugs that were already in the code, add clear comments, and correct only critical bugs. As far as that refinement went, there were no critical bugs that needed to be corrected.

AI was also used as a README engine. I wrote the raw ideas and explanations, then used AI to help arrange them into a structured, well-reasoned, and non-biased document. The physics project and the explanations behind it are mine; AI helped with organizing and refining the writing.

I could not afford richer AI coding tools such as Codex or Claude Code. The work was done using the AI tools that were already available to me. That is part of the context of the project, but it does not change who is responsible for the final work: I am, together with my team.

When I say “vibe-documented, not vibe-coded,” I mean AI helped improve code comments and documentation, but it did not independently come up with and build the whole project. AI did touch and refine some code, so I am not claiming that no AI was used in the source. The important point is that the project remained student-led, and the AI was given limited, specific instructions.

## My team’s contribution

There were four other teammates supporting me. They made changes to the code, with or without AI assistance. Some of that work was shared in separate folders and ZIP files instead of being pushed directly to GitHub.

I am keeping those files in a temporary directory until the whole project is complete and the files have been cleaned up and reviewed. After that, the plan is to integrate the changes and commit them to Git one by one, after small refinements. This should make it easier to see what changed and review the project history.

## What is and is not finished

The current project is not the final, complete version of the tool. There are more features that could be added. For example, support for other audio formats—including lossy formats—could be explored later.

That is not as simple as changing the file extension or copying the WAV approach. We need to understand how those formats store audio, how their headers or frames work, and how lossy encoding transforms the audio. MP3 support, for example, would need proper knowledge of its frame and metadata structures. The current WAV LSB method cannot be assumed to survive lossy encoding.

Those features are future possibilities, not something the current version already supports. They can be explored once we have the necessary format knowledge and can implement and test the changes properly.

## In short

AZORK is student-led work, inspired by wav-creator and supported by my four teammates. AI helped refine and comment on code under specific constraints, and helped structure my own raw writing into the README. I describe it as **vibe-documented, not vibe-coded**, while being transparent that AI did refine some code. The current version is still a work in progress, and the team is responsible for reviewing the code, documentation, and contributions before the project is submitted.

