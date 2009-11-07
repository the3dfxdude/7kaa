
# Set $wine_prefix to the prefix where wine is installed.
# This is typically "/usr".
$wine_prefix = "/usr";

# This sets up the include paths.
our @includes = (
"include",
"$wine_prefix/include/wine/windows",
"$wine_prefix/include/wine/msvcrt"
);
@includes = map { "-I$_" } @includes;

# The following sets flags used during compiling.
# The flags that are known to work:
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
