dnl Process this file with autoconf to produce a configure script
AC_INIT([escort], [0.2.4])
AC_CONFIG_AUX_DIR(.)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign no-define])
AC_PROG_YACC
AC_PROG_CXX
AC_PROG_CC
AM_PROG_AS
AC_PROG_RANLIB
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([
escort.spec
debian/changelog
Makefile
data/Makefile
data/16x16/Makefile
data/22x22/Makefile
data/24x24/Makefile
data/32x32/Makefile
data/48x48/Makefile
data/64x64/Makefile
data/128x128/Makefile
data/256x256/Makefile
data/escort/about/index.html
])

AH_BOTTOM([#include <Object.h>])
AH_BOTTOM([#if 4 < __GNUC__ || __GNUC__ == 4 && 6 <= __GNUC_MINOR__
#define HAVE_NULLPTR 1
#endif])

# Check for esidl 0.2.2 or later
AC_CHECK_PROG(HAVE_ESIDL, esidl, [`esidl --version`], [no], [])
if test "$HAVE_ESIDL" = "no"; then
	AC_MSG_ERROR([Cannot find esidl; esidl Web IDL compiler is needed.])
fi
ESIDL_VERSION=`esidl --version | cut -f 3 -d ' '`
AX_COMPARE_VERSION([$ESIDL_VERSION], [lt], [0.2.2],
	AC_MSG_ERROR([Cannot find esidl version 0.2.2 or later. A newer version is needed.]))

# check systems not following FHS (http://www.pathname.com/fhs)
AM_CONDITIONAL([HAVE_LIBEXEC], [test -d /usr/libexec])

# check for icu-config
AC_PATH_PROG(icu_config, icu-config, no)
if test "$icu_config" = "no"; then
	AC_MSG_ERROR([Cannot find icu-config. ICU library is needed.])
fi
ICU_CPPFLAGS=`$icu_config --cppflags`
ICU_LIBS=`$icu_config --ldflags`
AC_SUBST([ICU_CPPFLAGS])
AC_SUBST([ICU_LIBS])

# check for freetype-config
AC_PATH_PROG(freetype_config, freetype-config, no)
if test "$freetype_config" = "no"; then
	AC_MSG_ERROR([Cannot find freetype-config. FreeType library is needed.])
fi
FREETYPE_CPPFLAGS=`$freetype_config --cflags`
FREETYPE_LIBS=`$freetype_config --libs`
AC_SUBST([FREETYPE_CPPFLAGS])
AC_SUBST([FREETYPE_LIBS])

# check for SpiderMonkey 1.8.5
AC_SEARCH_LIBS(JS_NewCompartmentAndGlobalObject, js mozjs185)

# check for liberation fonts
AC_CHECK_FILE(/usr/share/fonts/liberation/LiberationSerif-Regular.ttf,[AC_SUBST(LIBERATON_TTF,/usr/share/fonts/liberation)])
AC_CHECK_FILE(/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf,[AC_SUBST(LIBERATON_TTF,/usr/share/fonts/truetype/liberation)])
AM_CONDITIONAL([HAVE_LIBERATON], [test x"LIBERATON_TTF" != x])

# check for DejaVu fonts
AC_CHECK_FILE(/usr/share/fonts/dejavu/DejaVuSerif.ttf,[AC_SUBST(DEJAVU_TTF,/usr/share/fonts/dejavu)])
AC_CHECK_FILE(/usr/share/fonts/truetype/ttf-dejavu/DejaVuSerif.ttf,[AC_SUBST(DEJAVU_TTF,/usr/share/fonts/truetype/ttf-dejavu)])
AM_CONDITIONAL([HAVE_DEJAVU], [test x"DEJAVU_TTF" != x])

# check for IPA fonts on Fedora
AC_CHECK_FILE(/usr/share/fonts/ipa-gothic/ipag.ttf,[AC_SUBST(HAVE_IPA_GOTHIC,/usr/share/fonts/ipa-gothic/ipag.ttf)])
AM_CONDITIONAL([HAVE_IPA_GOTHIC], [test x"HAVE_IPA_GOTHIC" != x])
AC_CHECK_FILE(/usr/share/fonts/ipa-mincho/ipam.ttf,[AC_SUBST(HAVE_IPA_MINCHO,/usr/share/fonts/ipa-mincho/ipam.ttf)])
AM_CONDITIONAL([HAVE_IPA_MINCHO], [test x"HAVE_IPA_MINCHO" != x])
AC_CHECK_FILE(/usr/share/fonts/ipa-pgothic/ipagp.ttf,[AC_SUBST(HAVE_IPA_PGOTHIC,/usr/share/fonts/ipa-pgothic/ipagp.ttf)])
AM_CONDITIONAL([HAVE_IPA_PGOTHIC], [test x"HAVE_IPA_PGOTHIC" != x])
AC_CHECK_FILE(/usr/share/fonts/ipa-pmincho/ipamp.ttf,[AC_SUBST(HAVE_IPA_PMINCHO,/usr/share/fonts/ipa-pmincho/ipamp.ttf)])
AM_CONDITIONAL([HAVE_IPA_PMINCHO], [test x"HAVE_IPA_PMINCHO" != x])

# check for IPA fonts on Ubuntu
AC_CHECK_FILE(/usr/share/fonts/truetype/takao-gothic/TakaoGothic.ttf,[AC_SUBST(HAVE_IPA_GOTHIC,/usr/share/fonts/truetype/takao-gothic/TakaoGothic.ttf)])
AC_CHECK_FILE(/usr/share/fonts/truetype/takao-mincho/TakaoMincho.ttf,[AC_SUBST(HAVE_IPA_MINCHO,/usr/share/fonts/truetype/takao-mincho/TakaoMincho.ttf)])
AC_CHECK_FILE(/usr/share/fonts/truetype/takao-gothic/TakaoPGothic.ttf,[AC_SUBST(HAVE_IPA_PGOTHIC,/usr/share/fonts/truetype/takao-gothic/TakaoPGothic.ttf)])
AC_CHECK_FILE(/usr/share/fonts/truetype/takao-mincho/TakaoPMincho.ttf,[AC_SUBST(HAVE_IPA_PMINCHO,/usr/share/fonts/truetype/takao-mincho/TakaoPMincho.ttf)])

# check for Aegean font
AC_CHECK_FILE(/usr/share/fonts/gdouros-aegean/Aegean.otf,[AC_SUBST(HAVE_AEGEAN,/usr/share/fonts/gdouros-aegean/Aegean.otf)])
AC_CHECK_FILE(/usr/share/fonts/truetype/ttf-ancient-scripts/Aegean600.ttf,[AC_SUBST(HAVE_AEGEAN,/usr/share/fonts/truetype/ttf-ancient-scripts/Aegean600.ttf)])
AM_CONDITIONAL([HAVE_AEGEAN], [test x"$HAVE_AEGEAN" != x])

AM_CONDITIONAL([TEST_FONTS], [test x"$TEST_FONTS" != x])

AC_OUTPUT
