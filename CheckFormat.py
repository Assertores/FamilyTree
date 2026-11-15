#!/usr/bin/env python3

# Checks the C and C++ files of the project with clang-format, e.g. from the pre-commit hook with
# `--files staged --fix`. Checks the same files as the CMake targets format and check-format
# (cmake/ClangFormat.cmake): the original implementation, the partial rewrite and spikes are left
# alone.

import argparse
import pathlib
import shutil
import subprocess
import sys

theExtensions = [".c", ".h", ".cpp", ".hpp"]
theFolders = ["library", "applications", "tests"]


def FileList(aArgs: argparse.Namespace):
	if aArgs.files == "all":
		# Run by hand
		return subprocess.check_output(["git", "ls-files"]).decode("utf-8").splitlines()
	if aArgs.files == "staged":
		# Git hook
		return subprocess.check_output(["git", "diff", "--name-only", "--diff-filter=d", "--cached"]).decode("utf-8").splitlines()
	if aArgs.source and aArgs.target:
		return subprocess.check_output(["git", "diff", "--name-only", "--diff-filter=d", aArgs.target, aArgs.source]).decode("utf-8").splitlines()
	raise Exception("invalid arguments: give --files, or --source and --target")


def IsFormatted(aPath: str):
	path = pathlib.PurePosixPath(aPath)
	return path.suffix in theExtensions and len(path.parts) > 1 and path.parts[0] in theFolders


def FindClangFormat():
	clangFormatPath = shutil.which("clang-format")
	if clangFormatPath:
		return clangFormatPath
	raise AssertionError("no clang-format detected.")


def main():
	parser = argparse.ArgumentParser(description="this tool checks for clang-format compliance")
	parser.add_argument("--source", help="the source branch to check from")
	parser.add_argument("--target", help="the target branch")
	parser.add_argument("--files", help="the list of files to check", choices=["all", "staged"])
	parser.add_argument("--fix", help="also apply the changes", action="store_true")
	args = parser.parse_args()

	clangFormatPath = FindClangFormat()
	subprocess.run([clangFormatPath, "--version"])
	print(flush=True)

	# git prints paths relative to the root of the repository.
	root = subprocess.check_output(["git", "rev-parse", "--show-toplevel"]).decode("utf-8").strip()

	exitcode = 0
	for file in filter(IsFormatted, FileList(args)):
		if subprocess.run([clangFormatPath, "--dry-run", "--Werror", "--ferror-limit=3", file], cwd=root).returncode != 0:
			exitcode = 1
			if args.fix:
				subprocess.run([clangFormatPath, "-i", "--Werror", file], cwd=root)
				print("FIXED", file, "- add it again before committing")
	print("DONE")
	return exitcode


if __name__ == "__main__":
	sys.exit(main())
