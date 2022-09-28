#! /bin/sh
# Do not edit this file!
# this file is generated by 'generate_test.pl'.

#
# The current language test.
# Determine the the current language from the configuration.
# The language name is an ISO-639-2 code and the corresponding
# ISO-639-1 code doesn't exist.
#
#             api: language
#            conf: language zUn
#  language_alias: 
#          expect: zun

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

rm -f idn2.conf
touch idn2.conf
echo 'language zUn' >> idn2.conf

touch ${SYSCONFDIR}/idnlang.conf || IGNORE=true
rm -f ${SYSCONFDIR}/idnlang.conf
( echo '' >> ${SYSCONFDIR}/idnlang.conf )

rm -f localmap1
rm -f localmap2
rm -f localmap3
rm -f localmap4
rm -f localset
rm -f expect.txt
rm -f output.txt

################## test ##################

echo 'zun' >> expect.txt
../common/test_language -conffile=idn2.conf > output.txt

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
rm -f ${SYSCONFDIR}/idnlang.conf
rm -f localmap1
rm -f localmap2
rm -f localmap3
rm -f localmap4
rm -f localset
rm -f expect.txt
rm -f output.txt
exit $RESULT