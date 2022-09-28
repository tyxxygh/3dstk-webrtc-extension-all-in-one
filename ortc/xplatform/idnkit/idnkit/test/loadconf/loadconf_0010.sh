#! /bin/sh
# Do not edit this file!
# this file is generated by 'generate_test.pl'.

#
# Configuration file loading test.
# Read user default configuration file.
#
#             api: encodename
#  local_encoding: U-escape
#      post_setup: [ -f "$SYSCONFDIR/idn2.conf" ] && IGNORE=true
#      post_setup: [ -f "${HOME}/.idn2rc" ] && IGNORE=true
#      post_setup: [ $IGNORE = false ] && cp -f $SRCDIR/no-newline.conf $HOME/.idn2rc || IGNORE=true
#    pre_teardown: [ $IGNORE = false ] && rm -f ${HOME}/.idn2rc
#            from: A@B
#         actions: IDN_ENCODE_REGIST
#          result: success
#              to: a.b

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
rm -f localmap1
rm -f localmap2
rm -f localmap3
rm -f localmap4
rm -f localset
rm -f expect.txt
rm -f output.txt

[ -f "$SYSCONFDIR/idn2.conf" ] && IGNORE=true
[ -f "${HOME}/.idn2rc" ] && IGNORE=true
[ $IGNORE = false ] && cp -f $SRCDIR/no-newline.conf $HOME/.idn2rc || IGNORE=true

################## test ##################

echo 'from: A@B' >> expect.txt
echo 'result: success' >> expect.txt
echo 'to: a.b' >> expect.txt
../common/test_encodename -conffile= -localcheckfile=  -encode-regist -- 'A@B' > output.txt

################## teardown ##################

[ $IGNORE = false ] && rm -f ${HOME}/.idn2rc

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
