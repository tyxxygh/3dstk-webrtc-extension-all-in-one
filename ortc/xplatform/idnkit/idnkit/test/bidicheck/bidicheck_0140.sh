#! /bin/sh
# Do not edit this file!
# this file is generated by 'generate_test.pl'.

#
# Bidicheck test.
# Tests domain names containing an A-label.
# The input name is U+00E0 + "." + U+05D0 (bidiclass: L . R).
#
#             api: decodename
#  local_encoding: U-escape
#            conf: 
#            from: xn--0ca.xn--4db
#         actions: IDN_DECODE_LOOKUP
#          result: success
#              to: \u{e0}.\u{5d0}

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

echo 'from: xn--0ca.xn--4db' >> expect.txt
echo 'result: success' >> expect.txt
echo 'to: \u{e0}.\u{5d0}' >> expect.txt
../common/test_decodename -conffile=idn2.conf -localcheckfile=  -decode-lookup -- 'xn--0ca.xn--4db' > output.txt

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
