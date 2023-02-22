FROM nixos/nix
MAINTAINER tlemane

RUN nix-channel --update

# TODO remove --impure when the release hash is known.
RUN nix --extra-experimental-features 'nix-command flakes' build github:tlemane/kmindex/dev --impure

WORKDIR /tmp

ENTRYPOINT ["/result/bin/kmindex"]
