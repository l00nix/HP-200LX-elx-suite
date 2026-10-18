# Publishing the prepared private repository

Prepared destination: **l00nix/elx-suite**, private. Initial suite release:
**v0.1.0**, marked as a prerelease because current builds await HP testing.

The source and documentation are ready locally. GitHub creation and release
upload were not completed: the terminal could not reach GitHub, and browser
access was declined. The connected GitHub account was still recognised.
Do not interpret local preparation as confirmation that the repository exists.

When GitHub access is available, use the normal authenticated GitHub workflow:

1. Check whether `l00nix/elx-suite` already exists. If so, inspect it and reuse
   only if appropriate; do not overwrite unrelated work.
2. Create the repository with **private** visibility. Do not enable public Pages.
3. Push this source tree and the `v0.1.0` tag. Verify the default README, all
   seven tool guides, PNG previews and SVG keycaps render on GitHub.
4. Publish a **prerelease** using `release-notes/v0.1.0.md` as its body.
5. Upload every file from local `release-assets/v0.1.0/`, including SHA256SUMS,
   the binary manifest, source archive and individual tool packages.
6. Verify private visibility and that all uploaded release-asset sizes/checksums
   match the local files. Keep binaries as release assets, out of source commits.

The top README uses the intended repository's Releases URL. Update it if a
different repository name is chosen before publishing.
