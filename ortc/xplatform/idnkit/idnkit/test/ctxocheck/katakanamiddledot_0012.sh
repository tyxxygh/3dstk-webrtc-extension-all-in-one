#! /bin/sh
# Do not edit this file!
# this file is generated by 'generate_test.pl'.

#
# KATAKANA MIDDLE DOT context rule test.
# The input name is U+0061 U+3042 U+30FB U+30A2 U+8A66.
#
#   U+0061 belongs to Common.
#   U+3042 belongs to Hiragana.
#   U+30A2 belongs to Katakana.
#   U+8A66 belongs to Han.
#
#             api: encodename
#  local_encoding: U-escape
#            conf: 
#            from: a\u{3042}\u{30fb}\u{30a2}\u{8a66}
#         actions: IDN_UNICODECONV IDN_CTXOCHECK IDN_LOCALCONV
#          result: success
#              to: a\u{3042}\u{30fb}\u{30a2}\u{8a66}

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
echo '' >> idn2.conf

rm -f localmap1
rm -f localmap2
rm -f localmap3
rm -f localmap4
rm -f localset
rm -f expect.txt
rm -f output.txt

################## test ##################

echo 'from: a\u{3042}\u{30fb}\u{30a2}\u{8a66}' >> expect.txt
echo 'result: success' >> expect.txt
echo 'to: a\u{3042}\u{30fb}\u{30a2}\u{8a66}' >> expect.txt
../common/test_encodename -conffile=idn2.conf -localcheckfile=  -unicodeconv -ctxocheck -localconv -- 'a\u{3042}\u{30fb}\u{30a2}\u{8a66}' > output.txt

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
