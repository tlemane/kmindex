{
  inputs = {
    nixpkgs = {
      url = "github:nixos/nixpkgs/nixos-unstable";
    };
    flake-utils = {
      url = "github:numtide/flake-utils";
    };
  };
  outputs = { self, nixpkgs, flake-utils, ... }: flake-utils.lib.eachSystem [
    "x86_64-linux"
  ] (system:
    let
      pkgs = import nixpkgs {
        inherit system;
      };

      kmindexBuildInputs = [
        pkgs.gcc12
        pkgs.cmake
        pkgs.zlib
        pkgs.gbenchmark
        pkgs.boost
        pkgs.bzip2
        pkgs.xxHash
        pkgs.nlohmann_json
        pkgs.fmt_8
        pkgs.gtest
        pkgs.spdlog
      ];

      kmindex = (with pkgs; stdenvNoCC.mkDerivation {
          pname = "kmindex";
          version = "0.3.0";
          src = builtins.fetchGit {
            url = "https://github.com/tlemane/kmindex";
            rev = "411474c7a63d0dba56b034210cc4fc9ae56ee1a7";
            submodules = true;
          };
          nativeBuildInputs = kmindexBuildInputs;

          configurePhase = ''
            cmake -S . -B build
          '';

          buildPhase = ''
            cmake --build ./build
          '';

          installPhase = ''
            mkdir -p $out/bin
            cp ./build/app/kmindex/kmindex $out/bin
            cp ./build/app/kmindex-server/kmindex-server $out/bin
          '';
        }
      );
    in rec {
      defaultApp = flake-utils.lib.mkApp {
        drv = defaultPackage;
      };
      defaultPackage = kmindex;

      devShell = pkgs.mkShell {
        name = "kmindex_dev_env";
        buildInputs = kmindexBuildInputs;
      };

      apps.default = {
        type = "app";
        program = "${self.defaultPackage.${system}}/bin/kmindex";
      };
    }
  );
}
