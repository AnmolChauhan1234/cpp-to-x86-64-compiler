{
  description = "C++ compiler project with CMake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in {
        devShells.default = pkgs.mkShell {
          buildInputs = [
            pkgs.cmake
            pkgs.clang
            pkgs.lldb
            pkgs.gnumake
          ];

          CXX = "clang++";
          CC = "clang";
          CXXFLAGS = "-std=c++20 -Wall -Wextra -Werror";
        };
      });
}
