# Code Quality
At BioDynaMo we are aiming to develop a high quality software product. The following practices help us to achieve this goal:

* C++ Style Guide
* Documentation (User guide, Developer guide, API)
* Git Conventions
* Test driven development (TDD)
* Continues integration (CI)

## Code Styleguide

A coding styleguide is a set of guidelines and best practices which improve readability and maintainability of a code base. Code is more often read than rewritten. Therefore it is important that a developer quickly understands a piece of code even if it was written by someone else. A coding standard helps to achieve that. We are using the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)

## Test Driven Development (TDD) and Continues Integration (CI)

TDD and CI are two practices from agile development. Test Driven Development is a special way of using unit tests. A unit test is a piece of code that tests a certain functionality of our application. If we make some changes in the code and at the end all unit tests pass, we most likely did not break something. This increases confidence in the code and reduces the fear to "touch" others code.
Continues Integration takes all this automated tests and executes them on every change to the code repository. We are using [Travis-CI](https://travis-ci.org/BioDynaMo/biodynamo)

More information:

* [Intro to TDD](https://www.youtube.com/watch?v=QCif_-r8eK4)
* Beck, Kent. Extreme programming explained: embrace change. addison-wesley professional, 2000.

## Git Conventions

### Git Workflow

We are following the Git workflow proposed by Vincent Driessen in his blog post: [A successful Git branching model](http://nvie.com/posts/a-successful-git-branching-model/) with the modification that his `develop` branch is our `master` branch.

### Git Commit Message

Taken from a great blog post from [Chris Beams](http://chris.beams.io/posts/git-commit/)

1. Separate subject from body with a blank line
2. Capitalize the subject line
3. Do not end the subject line with a period
4. Use the imperative mood in the subject line
5. Use the body to explain what and why vs. how
6. Limit the subject line to 50 characters
7. Wrap the body at 72 characters

```
Summarize changes in around 50 characters or less

More detailed explanatory text, if necessary. Wrap it to about 72
characters or so. In some contexts, the first line is treated as the
subject of the commit and the rest of the text as the body. The
blank line separating the summary from the body is critical (unless
you omit the body entirely); various tools like `log`, `shortlog`
and `rebase` can get confused if you run the two together.

Explain the problem that this commit is solving. Focus on why you
are making this change as opposed to how (the code explains that).
Are there side effects or other unintuitive consequenses of this
change? Here's the place to explain them.

Further paragraphs come after blank lines.

 - Bullet points are okay, too

 - Typically a hyphen or asterisk is used for the bullet, preceded
   by a single space, with blank lines in between, but conventions
   vary here

If you use an issue tracker, put references to them at the bottom,
like this:

Resolves: #123
See also: #456, #789
```

Use `git commit` without the `-m` switch to write a commit body.

### Master Branch

Each commit in the master branch should pass the CI build. Therefore all development should be carried out in a feature/hotfix branch. Once development has been finished and the build passes, it can be merged back into `master`.
