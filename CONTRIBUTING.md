# Contributing

Guidelines partially taken from [OpenMW](https://github.com/OpenMW/openmw) CONTRIBUTING.md.

Feel free to open an issue, pull request, or comment on a commit.

## Vision
As the XL Engine was originally developed by LuciusDXL, contributors should keep these points in mind to help maintain the original direction of the project:
- Support Daggerfall, Dark Forces, Blood, and Outlaws (Shadow Warrior?)
- Each game's behavior should match the original game as closely as possible while fixing obvious crash bugs, etc.

## Pull Requests
Pull request descriptions should include the following (if applicable):
- Summary of changes made
- Reasoning/motivation/objective behind changes
- Any testing carried out to verify changes

Pull requests themselves should follow these guidelines:
- Avoid making changes directly to `master`. Instead, make a temporary branch with a descriptive name (all lowercase with dashes for spaces; i.e., `example-branch-name`), and use that with the pull request instead.
- Each feature/bug-fix should go into a separate PR for ease of merging, unless they are closely related or dependent upon each other.
- Feel free to submit incomplete pull requests. Even if the work cannot be merged yet, pull requests are a great place to collect early feedback. Just make sure to mark it as **[Incomplete]** or **[Don't Merge]** in the title.

## Original engine "bug" fixes
From time to time you may be tempted to "fix" what you think was a "bug" in the original game engine.

Unfortunately, the definition of what is a "bug" is not so clear. Consider that your "bug" is actually a feature unless proven otherwise:
- We have no way of knowing what the original developers really intended (short of asking them, good luck with that).
- What may seem like an illogical mechanic can actually be part of an attempt to balance the game. 
- Many people will actually <i>like</i> these "bugs" because that is what they remember the game for.
- Exploits may be part of the fun of an open-world game - they reward knowledge with power. There are too many of them to plug them all, anyway.

The XL Engine, in its default configuration, is meant to be a faithful re-implementation of several engines, minus things like crash bugs, stability issues, and design errors. However, we try to avoid touching anything that affects the core gameplay, the balancing of the game, or introduces incompatibilities with existing mod content.

That said, we may sometimes evaluate such issues on an individual basis. Common exceptions to the above would be:
- Issues so glaring that they would severely limit the capabilities of the engine in the future
- Bugs where the intent is very obvious, and have little to no balancing impact
- Bugs that were fixed in an official patch
