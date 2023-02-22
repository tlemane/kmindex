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
        pkgs.gcc10
        pkgs.cmake
        pkgs.zlib
        pkgs.gbenchmark
        pkgs.boost
        pkgs.bzip2
        pkgs.xxHash
        pkgs.nolhmann_json
        pkgs.fmt_9
      ];

      kmindex = (with pkgs; stdenv.mkDerivation {
          pname = "kmindex";
          version = "0.1.0";
          src = fetchgit {
            url = "https://github.com/tlemane/kmindex";
            rev = "dev";
            sha256 = "sha256-9WmdgUYLgQfJsn+TcQtj4njwkGTM+EHeqaX7EVmT6lc=";
            fetchSubmodules = true;
          };
          nativeBuildInputs = kmindexBuildInputs;

          configurePhase = ''

          '';

          buildPhase = ''
            bash install.sh
          '';

          installPhase = ''
            mkdir -p $out/bin
            cp $TMP/kmindex/kmindex_install/bin/* $out/bin
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
