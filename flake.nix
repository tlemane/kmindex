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
    "x86_64-linux" "x86_64-darwin"
  ] (system:
    let
      pkgs = import nixpkgs {
        inherit system;
      };

      kmindexBuildInputs = [
        pkgs.gcc11
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
          version = "0.2.0";
          src = builtins.fetchGit {
            url = "https://github.com/tlemane/kmindex";
            ref = "dev";
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
