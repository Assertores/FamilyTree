#!/usr/bin/env python

import argparse
import pathlib
import shutil
import subprocess
import sys

def FileList(aArgs):
	if aArgs.files == "all":
		# Run by hand
		return subprocess.check_output(["git", "ls-files"]).decode("utf-8").splitlines()
	if aArgs.files == "staged":
		# Git Hook
		return subprocess.check_output(["git", "diff", "--name-only", "--diff-filter=d", "--cached"]).decode("utf-8").splitlines()

	targetBranchName = ""
	sourceBranchName = ""
	if aArgs.source and aArgs.target:
		sourceBranchName = aArgs.source
		targetBranchName = aArgs.target
	else:
		raise Exception("invalide arguments")
	return subprocess.check_output(["git", "diff", "--name-only", "--diff-filter=d", targetBranchName, sourceBranchName]).decode("utf-8").splitlines()

def FindClangFormat():
	clangFormatPath = shutil.which("clang-format")
	if clangFormatPath:
		return clangFormatPath

	raise AssertionError("no clang-format detected.")


def main():
	parser = argparse.ArgumentParser(description="this tool checks for clang-format complience")
	parser.add_argument("--source", help="the source branch to check from")
	parser.add_argument("--target", help="targetbranch")
	parser.add_argument("--files", help="the list of files to check", choices=["all", "staged"])
	parser.add_argument("--fix", help="if this is set to true it will also aply the changes", action='store_true')
	args = parser.parse_args()

	clangFormatPath = FindClangFormat()

	print("File Last Changed in commit:", flush=True)
	# subprocess.run(["git", "log", "-n" , "1", pathlib.Path(__file__).name], cwd=os.path.dirname(__file__))
	subprocess.run([clangFormatPath, "--version"])
	print(flush=True)

	changedFiles = FileList(args)

	exitcode = 0
	for file in filter(lambda fileString : pathlib.Path(fileString).suffix in [".h", ".cpp", ".proto"], changedFiles):
		if subprocess.run([clangFormatPath, "--dry-run", "--Werror", "--ferror-limit=3", file]).returncode != 0:
			exitcode = 1
			if args.fix:
				subprocess.run([clangFormatPath, "-i", "--Werror", file])
				print("FIXED")
	print("DONE")
	return exitcode

if __name__ == "__main__":
	sys.exit(main())
