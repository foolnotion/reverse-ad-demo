{
  description = "Reverse-AD Devenv";

  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:nixos/nixpkgs/master";
    pratt-parser.url = "github:foolnotion/pratt-parser-calculator";
    foolnotion.url = "github:foolnotion/nur-pkg";
  };

  outputs =
    {
      self,
      flake-utils,
      nixpkgs,
      foolnotion,
      pratt-parser,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ foolnotion.overlay ];
        };

      in
      rec {
        devShells.default = pkgs.llvmPackages_latest.stdenv.mkDerivation {
          name = "reverse-ad";

          nativeBuildInputs =
            with pkgs;
            [
              cmake
              ninja
              clang-tools
              codespell
              cppcheck
              include-what-you-use
            ]
            ++ pkgs.lib.optionals (pkgs.stdenv.hostPlatform.system == "x86_64-linux") [ pkgs.gcc15 ];

          buildInputs =
            with pkgs;
            [
              # dependencies
              eigen
              xxhash_cpp
              unordered_dense
            ]
            ++ pkgs.lib.optionals (pkgs.stdenv.hostPlatform.system == "x86_64-linux") [
              # debugging and profiling
              gdb
              graphviz
              hotspot
              hyperfine
              perf
              pyprof2calltree
              seer
              valgrind
            ];

          shellHook = ''
            alias bb="cmake --build build -j"
          '';
        };
      }
    );
}
