# Publishing releases

Repository: **l00nix/HP-200LX-elx-suite**, public. Keep current experimental builds
marked as prereleases until physical HP validation supports a stable release.

Package 0.2.1 contains only the four HP firmware tools. Public EPT builds use
this repository. Historical commits and releases remain available; the split
of Cmdr and scrcap applies to the current source and packages from 0.2.0 onward.

1. Update tool guides, preview captions, testing status and release notes.
2. Run the relevant tests and `python3 scripts/check_docs.py`.
3. Commit source and documentation; create the version tag at that commit.
4. Prepare binaries and source archives in the ignored `release-assets/` folder.
   Include a binary manifest, source commit ID, licenses and SHA256SUMS.
5. Push the commit and tag, then create the GitHub prerelease using its release
   notes. Upload the prepared assets; keep binaries out of source commits.
6. Verify repository visibility is public, remote source matches the tagged
   commit, and downloaded release assets match the local checksums.

Keep physical testing separate from mocked and emulator checks. Each preview should
identify its origin and tool version.
