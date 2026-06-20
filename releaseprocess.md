# Release Process

- Bump LIBHATCHET_VER, LIBHATCHET_TAG and remove -dev.

```sh
./testall.sh
git add .
git commit -m v1.xx.0
git checkout main
git merge dev
git push
git tag v1.xx.0
git push --tags
git checkout dev
```

- Bump LIBHATCHET_VER, LIBHATCHET_TAG and add -dev.
- Do GitHub Release.
