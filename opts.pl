$wine_include_path = "/usr/local/include/wine";

our @includes = (
"include",
"$wine_include_path/windows",
"$wine_include_path/msvcrt"
);
@includes = map { "-I$_" } @includes;

# AMPLUS
# DISABLE_MULTI_PLAYER
# FRENCH
# GERMAN
# SPANISH
our @defines = qw(
AMPLUS
DISABLE_MULTI_PLAYER
);
@defines = map { "-D$_" } @defines;

1;
