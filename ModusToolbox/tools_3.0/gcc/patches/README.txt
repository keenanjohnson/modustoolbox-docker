Newlib patches:

- fix-newlib-stdio-memleak.patch

Reason: https://ecos.sourceware.org/ml/newlib/current/017697.html
Status: mailing list review

GDB patches:

- gdbsupport-Use-LOCALAPPDATA-to-determine-cache-dir.patch

Reason: https://github.com/Marus/cortex-debug/issues/199
https://sourceware.org/pipermail/gdb-patches/2020-December/173903.html
Status: picked from GDB 10.2 mainline:
https://sourceware.org/git/?p=binutils-gdb.git;a=commitdiff;h=60a7223fdd196d540f87504833166f558c95c035
Backported to GDB 10.1 (tweaked entry order in gdbsupport/ChangeLog).

