with import <nixpkgs> {};
stdenv.mkDerivation rec {
  name = "xhotkey";
  buildInputs = [ xlibsWrapper ];

  #src = fetchgit {
  #  url = "https://github.com/mnakama/${name}";
  #  rev = "56d47de794ef0c818f582cf407ff2a3c7ae9b930";
  #  sha256 = "193rifhpgfyma2b49dfx94gmkgvsv0qmfsm2s43dp1fn7kbpr07r";
  #};

  src = ".";

  unpackPhase = ''
    cp ${./Makefile} Makefile
    cp ${./xhotkey.c} xhotkey.c
  '';

  dontConfigure = true;

  installPhase = ''
    mkdir -p $out/bin
    cp ./xhotkey $out/bin/xhotkey
  '';
}
