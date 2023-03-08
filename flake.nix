{
  description = "Reverse-AD Devenv";

  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:nixos/nixpkgs/master";
    pratt-parser.url = "github:foolnotion/pratt-parser-calculator";
    foolnotion.url = "github:foolnotion/nur-pkg";
  };

  outputs = { self, flake-utils, nixpkgs, foolnotion, pratt-parser }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ foolnotion.overlay ];
        };

        stdenv_ = pkgs.overrideCC pkgs.llvmPackages_15.stdenv (
          pkgs.clang_15.override { gccForLibs = pkgs.gcc12.cc; }
        );

      in rec {
        devShells.default = stdenv_.mkDerivation {
          name = "reverse-ad";

          nativeBuildInputs = operon.nativeBuildInputs ++ (with pkgs; [
            cmake
            ninja
            clang-tools
            cppcheck
            include-what-you-use
          ]);

          buildInputs = with pkgs; [
            # dependencies
            eigen

            # debugging and profiling
            gdb
            graphviz
            hotspot
            hyperfine
            linuxPackages.perf
            pyprof2calltree
            qcachegrind
            seer
            valgrind
          ];

          shellHook = ''
            alias bb="cmake --build build -j"
          '';
        };
      });
}
