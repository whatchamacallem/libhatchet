# Guidelines

## Release Process

- First bump LIBHATCHET_VER, LIBHATCHET_TAG and remove -dev.
- Then run the following:

```sh
TAG=v1.xx.0
./testall.sh
git add .
git commit -m $TAG
git checkout main
git merge --squash dev
git commit -m $TAG
git push
git tag $TAG
git push --tags
git checkout dev
git reset --hard main
git push --force
```

- Again, bump LIBHATCHET_VER, LIBHATCHET_TAG and add -dev back.
- Finally do GitHub Release.

## Git

- The stable release branch is `main`. Branch off it as `dev` is not guaranteed to be stable.
- Put `+symbol` or `+file` in the commit message when adding a new symbol or file.

How to use worktrees for multitasking:

`git worktree add -b <new-branch-name> worktree/<path> <base-branch>`

## Code of conduct

May you do good and not evil. May you find forgiveness for yourself and forgive
others. May you share freely, never taking more than you give.
