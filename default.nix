with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "xhotkey";
  buildInputs = [ xlibsWrapper ];
}
