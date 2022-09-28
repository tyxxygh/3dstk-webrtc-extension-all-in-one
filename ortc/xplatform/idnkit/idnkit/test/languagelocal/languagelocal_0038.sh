#! /bin/sh
# Do not edit this file!
# this file is generated by 'generate_test.pl'.

#
# language-local conversion test.
# The map result contains U+D7FF.
#
#             api: encodename
#  local_encoding: U-escape
#            conf: map language-local
#            conf: language-local * @LOCALMAP_FILE1@
#       localmap1: 0041; D7FF;
#            from: ABC
#         actions: IDN_UNICODECONV IDN_MAP IDN_LOCALCONV
#          result: success
#              to: \u{d7ff}BC

################## setup ##################

unset LC_ALL
unset LC_CTYPE
unset LC_MESSAGES
unset LANG
unset IDN_LOCAL_CODESET
unset IDN_LOG_LEVEL

SRCDIR=`dirname $0`
SYSCONFDIR=${SYSCONFDIR-"/etc"}
IGNORE=false
export IDN_LOCAL_CODESET; IDN_LOCAL_CODESET=U-escape

rm -f idn2.conf
touch idn2.conf
echo 'map language-local' >> idn2.conf
echo 'language-local * localmap1' >> idn2.conf

rm -f localmap1
echo '0041; D7FF;' >> localmap1

rm -f localmap2
rm -f localmap3
rm -f localmap4
rm -f localset
rm -f expect.txt
rm -f output.txt

################## test ##################

echo 'from: ABC' >> expect.txt
echo 'result: success' >> expect.txt
echo 'to: \u{d7ff}BC' >> expect.txt
../common/test_encodename -conffile=idn2.conf -localcheckfile=  -unicodeconv -map -localconv -- 'ABC' > output.txt

################## teardown ##################

cmp expect.txt output.txt > /dev/null 2>&1
RESULT=$?
[ X$IGNORE = Xtrue ] && RESULT=77
if [ X$QUIET != Xtrue ]; then
    if [ $RESULT -eq 0 ]; then
        echo "PASS: $0"
    elif [ $RESULT -eq 77 ]; then
        echo "SKIP: $0"
    else
        echo "FAIL: $0"
    fi
fi
[ $RESULT -eq 0 -o $RESULT -eq 77 ] || exit $RESULT
rm -f idn2.conf
rm -f localmap1
rm -f localmap2
rm -f localmap3
rm -f localmap4
rm -f localset
rm -f expect.txt
rm -f output.txt
exit $RESULT
