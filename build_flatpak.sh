#!/bin/bash

# Define variables
BUILD_DIR="build-dir"
MANIFEST="dev.rotstein.packagemanager.json"

# Clean previous build
rm -rf $BUILD_DIR
rm -rf sassc  # Remove the existing sassc directory

# Update the URL to the new repository
git_clone_url="https://github.com/sass/sassc.git"
git_branch="master"  # Use the correct branch name

# Clone the repository
git clone --depth 1 --branch $git_branch $git_clone_url sassc

# Build the Flatpak
flatpak-builder --force-clean $BUILD_DIR $MANIFEST

# Install the Flatpak
flatpak-builder --user --install --force-clean $BUILD_DIR $MANIFEST

# Run the Flatpak
flatpak run dev.rotstein.packagemanager