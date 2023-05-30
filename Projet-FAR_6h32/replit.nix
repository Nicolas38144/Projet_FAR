{ pkgs }: {
	deps = [
		pkgs.iproute2
  pkgs.clang_12
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
	];
}