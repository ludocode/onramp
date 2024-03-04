#!/bin/sh

# Copyright (c) 2023-2024 Fraser Heavy Software
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

########################################################################

# This is an alternate implementation of the Onramp hex tool in POSIX shell
# that does not use arithmetic expressions nor parameter expansion.
#
# It is much slower than other implementations (especially in Bash) but it does
# work. It may be more portable because it has minimal dependencies.
#
# It does not do much error checking. It will recognize truncated hex bytes but
# that's about it. Any other characters will be treated as whitespace.

# Check for too many arguments
if ! [ -z "$4" ]; then
    echo "Usage: $0 <input_file> -o <output_file>"
    exit 1
fi

# Allow -o before and after the input file
if [ "$1" = "-o" ]; then
    o="$2"
    i="$3"
elif [ "$2" = "-o" ]; then
    i="$1"
    o="$3"
else
    echo "Usage: $0 <input_file> -o <output_file>"
    exit 1
fi

# Prepare the output
rm -f "$o" 2>/dev/null
if ! touch "$o"; then
    echo "ERROR: Failed to create output file." >&2
    exit 1
fi

while IFS= read line || [ -n "$line" ]; do
    while [ -n "$line" ]; do
        c=$(echo "$line"|cut -c 1)

        # Check for a hex byte
        ishex=0
        case "0123456789ABCDEFabcdef" in
            *$c*) ishex=1 ;;
        esac
        if [ $ishex -eq 1 ]; then

            hi=$(echo "$line"|cut -c 1)
            lo=$(echo "$line"|cut -c 2)
            line=$(echo "$line"|cut -c 3-)

            # Convert to uppercase
            case "$hi" in a) hi=A ;; b) hi=B ;; c) hi=C ;; d) hi=D ;; e) hi=E ;; f) hi=F ;; esac
            case "$lo" in a) lo=A ;; b) lo=B ;; c) lo=C ;; d) lo=D ;; e) lo=E ;; f) lo=F ;; esac
            hex=$hi$lo

            case "$hex" in
                00) printf   '\0' >> "$o" ;; 01) printf   '\1' >> "$o" ;; 02) printf   '\2' >> "$o" ;; 03) printf   '\3' >> "$o" ;;
                04) printf   '\4' >> "$o" ;; 05) printf   '\5' >> "$o" ;; 06) printf   '\6' >> "$o" ;; 07) printf   '\7' >> "$o" ;;
                08) printf  '\10' >> "$o" ;; 09) printf  '\11' >> "$o" ;; 0A) printf  '\12' >> "$o" ;; 0B) printf  '\13' >> "$o" ;;
                0C) printf  '\14' >> "$o" ;; 0D) printf  '\15' >> "$o" ;; 0E) printf  '\16' >> "$o" ;; 0F) printf  '\17' >> "$o" ;;
                10) printf  '\20' >> "$o" ;; 11) printf  '\21' >> "$o" ;; 12) printf  '\22' >> "$o" ;; 13) printf  '\23' >> "$o" ;;
                14) printf  '\24' >> "$o" ;; 15) printf  '\25' >> "$o" ;; 16) printf  '\26' >> "$o" ;; 17) printf  '\27' >> "$o" ;;
                18) printf  '\30' >> "$o" ;; 19) printf  '\31' >> "$o" ;; 1A) printf  '\32' >> "$o" ;; 1B) printf  '\33' >> "$o" ;;
                1C) printf  '\34' >> "$o" ;; 1D) printf  '\35' >> "$o" ;; 1E) printf  '\36' >> "$o" ;; 1F) printf  '\37' >> "$o" ;;
                20) printf  '\40' >> "$o" ;; 21) printf  '\41' >> "$o" ;; 22) printf  '\42' >> "$o" ;; 23) printf  '\43' >> "$o" ;;
                24) printf  '\44' >> "$o" ;; 25) printf  '\45' >> "$o" ;; 26) printf  '\46' >> "$o" ;; 27) printf  '\47' >> "$o" ;;
                28) printf  '\50' >> "$o" ;; 29) printf  '\51' >> "$o" ;; 2A) printf  '\52' >> "$o" ;; 2B) printf  '\53' >> "$o" ;;
                2C) printf  '\54' >> "$o" ;; 2D) printf  '\55' >> "$o" ;; 2E) printf  '\56' >> "$o" ;; 2F) printf  '\57' >> "$o" ;;
                30) printf  '\60' >> "$o" ;; 31) printf  '\61' >> "$o" ;; 32) printf  '\62' >> "$o" ;; 33) printf  '\63' >> "$o" ;;
                34) printf  '\64' >> "$o" ;; 35) printf  '\65' >> "$o" ;; 36) printf  '\66' >> "$o" ;; 37) printf  '\67' >> "$o" ;;
                38) printf  '\70' >> "$o" ;; 39) printf  '\71' >> "$o" ;; 3A) printf  '\72' >> "$o" ;; 3B) printf  '\73' >> "$o" ;;
                3C) printf  '\74' >> "$o" ;; 3D) printf  '\75' >> "$o" ;; 3E) printf  '\76' >> "$o" ;; 3F) printf  '\77' >> "$o" ;;
                40) printf '\100' >> "$o" ;; 41) printf '\101' >> "$o" ;; 42) printf '\102' >> "$o" ;; 43) printf '\103' >> "$o" ;;
                44) printf '\104' >> "$o" ;; 45) printf '\105' >> "$o" ;; 46) printf '\106' >> "$o" ;; 47) printf '\107' >> "$o" ;;
                48) printf '\110' >> "$o" ;; 49) printf '\111' >> "$o" ;; 4A) printf '\112' >> "$o" ;; 4B) printf '\113' >> "$o" ;;
                4C) printf '\114' >> "$o" ;; 4D) printf '\115' >> "$o" ;; 4E) printf '\116' >> "$o" ;; 4F) printf '\117' >> "$o" ;;
                50) printf '\120' >> "$o" ;; 51) printf '\121' >> "$o" ;; 52) printf '\122' >> "$o" ;; 53) printf '\123' >> "$o" ;;
                54) printf '\124' >> "$o" ;; 55) printf '\125' >> "$o" ;; 56) printf '\126' >> "$o" ;; 57) printf '\127' >> "$o" ;;
                58) printf '\130' >> "$o" ;; 59) printf '\131' >> "$o" ;; 5A) printf '\132' >> "$o" ;; 5B) printf '\133' >> "$o" ;;
                5C) printf '\134' >> "$o" ;; 5D) printf '\135' >> "$o" ;; 5E) printf '\136' >> "$o" ;; 5F) printf '\137' >> "$o" ;;
                60) printf '\140' >> "$o" ;; 61) printf '\141' >> "$o" ;; 62) printf '\142' >> "$o" ;; 63) printf '\143' >> "$o" ;;
                64) printf '\144' >> "$o" ;; 65) printf '\145' >> "$o" ;; 66) printf '\146' >> "$o" ;; 67) printf '\147' >> "$o" ;;
                68) printf '\150' >> "$o" ;; 69) printf '\151' >> "$o" ;; 6A) printf '\152' >> "$o" ;; 6B) printf '\153' >> "$o" ;;
                6C) printf '\154' >> "$o" ;; 6D) printf '\155' >> "$o" ;; 6E) printf '\156' >> "$o" ;; 6F) printf '\157' >> "$o" ;;
                70) printf '\160' >> "$o" ;; 71) printf '\161' >> "$o" ;; 72) printf '\162' >> "$o" ;; 73) printf '\163' >> "$o" ;;
                74) printf '\164' >> "$o" ;; 75) printf '\165' >> "$o" ;; 76) printf '\166' >> "$o" ;; 77) printf '\167' >> "$o" ;;
                78) printf '\170' >> "$o" ;; 79) printf '\171' >> "$o" ;; 7A) printf '\172' >> "$o" ;; 7B) printf '\173' >> "$o" ;;
                7C) printf '\174' >> "$o" ;; 7D) printf '\175' >> "$o" ;; 7E) printf '\176' >> "$o" ;; 7F) printf '\177' >> "$o" ;;
                80) printf '\200' >> "$o" ;; 81) printf '\201' >> "$o" ;; 82) printf '\202' >> "$o" ;; 83) printf '\203' >> "$o" ;;
                84) printf '\204' >> "$o" ;; 85) printf '\205' >> "$o" ;; 86) printf '\206' >> "$o" ;; 87) printf '\207' >> "$o" ;;
                88) printf '\210' >> "$o" ;; 89) printf '\211' >> "$o" ;; 8A) printf '\212' >> "$o" ;; 8B) printf '\213' >> "$o" ;;
                8C) printf '\214' >> "$o" ;; 8D) printf '\215' >> "$o" ;; 8E) printf '\216' >> "$o" ;; 8F) printf '\217' >> "$o" ;;
                90) printf '\220' >> "$o" ;; 91) printf '\221' >> "$o" ;; 92) printf '\222' >> "$o" ;; 93) printf '\223' >> "$o" ;;
                94) printf '\224' >> "$o" ;; 95) printf '\225' >> "$o" ;; 96) printf '\226' >> "$o" ;; 97) printf '\227' >> "$o" ;;
                98) printf '\230' >> "$o" ;; 99) printf '\231' >> "$o" ;; 9A) printf '\232' >> "$o" ;; 9B) printf '\233' >> "$o" ;;
                9C) printf '\234' >> "$o" ;; 9D) printf '\235' >> "$o" ;; 9E) printf '\236' >> "$o" ;; 9F) printf '\237' >> "$o" ;;
                A0) printf '\240' >> "$o" ;; A1) printf '\241' >> "$o" ;; A2) printf '\242' >> "$o" ;; A3) printf '\243' >> "$o" ;;
                A4) printf '\244' >> "$o" ;; A5) printf '\245' >> "$o" ;; A6) printf '\246' >> "$o" ;; A7) printf '\247' >> "$o" ;;
                A8) printf '\250' >> "$o" ;; A9) printf '\251' >> "$o" ;; AA) printf '\252' >> "$o" ;; AB) printf '\253' >> "$o" ;;
                AC) printf '\254' >> "$o" ;; AD) printf '\255' >> "$o" ;; AE) printf '\256' >> "$o" ;; AF) printf '\257' >> "$o" ;;
                B0) printf '\260' >> "$o" ;; B1) printf '\261' >> "$o" ;; B2) printf '\262' >> "$o" ;; B3) printf '\263' >> "$o" ;;
                B4) printf '\264' >> "$o" ;; B5) printf '\265' >> "$o" ;; B6) printf '\266' >> "$o" ;; B7) printf '\267' >> "$o" ;;
                B8) printf '\270' >> "$o" ;; B9) printf '\271' >> "$o" ;; BA) printf '\272' >> "$o" ;; BB) printf '\273' >> "$o" ;;
                BC) printf '\274' >> "$o" ;; BD) printf '\275' >> "$o" ;; BE) printf '\276' >> "$o" ;; BF) printf '\277' >> "$o" ;;
                C0) printf '\300' >> "$o" ;; C1) printf '\301' >> "$o" ;; C2) printf '\302' >> "$o" ;; C3) printf '\303' >> "$o" ;;
                C4) printf '\304' >> "$o" ;; C5) printf '\305' >> "$o" ;; C6) printf '\306' >> "$o" ;; C7) printf '\307' >> "$o" ;;
                C8) printf '\310' >> "$o" ;; C9) printf '\311' >> "$o" ;; CA) printf '\312' >> "$o" ;; CB) printf '\313' >> "$o" ;;
                CC) printf '\314' >> "$o" ;; CD) printf '\315' >> "$o" ;; CE) printf '\316' >> "$o" ;; CF) printf '\317' >> "$o" ;;
                D0) printf '\320' >> "$o" ;; D1) printf '\321' >> "$o" ;; D2) printf '\322' >> "$o" ;; D3) printf '\323' >> "$o" ;;
                D4) printf '\324' >> "$o" ;; D5) printf '\325' >> "$o" ;; D6) printf '\326' >> "$o" ;; D7) printf '\327' >> "$o" ;;
                D8) printf '\330' >> "$o" ;; D9) printf '\331' >> "$o" ;; DA) printf '\332' >> "$o" ;; DB) printf '\333' >> "$o" ;;
                DC) printf '\334' >> "$o" ;; DD) printf '\335' >> "$o" ;; DE) printf '\336' >> "$o" ;; DF) printf '\337' >> "$o" ;;
                E0) printf '\340' >> "$o" ;; E1) printf '\341' >> "$o" ;; E2) printf '\342' >> "$o" ;; E3) printf '\343' >> "$o" ;;
                E4) printf '\344' >> "$o" ;; E5) printf '\345' >> "$o" ;; E6) printf '\346' >> "$o" ;; E7) printf '\347' >> "$o" ;;
                E8) printf '\350' >> "$o" ;; E9) printf '\351' >> "$o" ;; EA) printf '\352' >> "$o" ;; EB) printf '\353' >> "$o" ;;
                EC) printf '\354' >> "$o" ;; ED) printf '\355' >> "$o" ;; EE) printf '\356' >> "$o" ;; EF) printf '\357' >> "$o" ;;
                F0) printf '\360' >> "$o" ;; F1) printf '\361' >> "$o" ;; F2) printf '\362' >> "$o" ;; F3) printf '\363' >> "$o" ;;
                F4) printf '\364' >> "$o" ;; F5) printf '\365' >> "$o" ;; F6) printf '\366' >> "$o" ;; F7) printf '\367' >> "$o" ;;
                F8) printf '\370' >> "$o" ;; F9) printf '\371' >> "$o" ;; FA) printf '\372' >> "$o" ;; FB) printf '\373' >> "$o" ;;
                FC) printf '\374' >> "$o" ;; FD) printf '\375' >> "$o" ;; FE) printf '\376' >> "$o" ;; FF) printf '\377' >> "$o" ;;
                *)
                    echo "ERROR: Unrecognized hex byte: $hex" >&2
                    exit 1
                    ;;
            esac
        elif [ "x$c" = "x#" ] || [ "x$c" = "x;" ] || [ "x$c" = "x@" ]; then
            # Comment (or address assertion), discard the rest of the line
            line=""
        else
            # Assume it's whitespace, ignore it
            line=$(echo "$line"|cut -c 2-)
        fi
    done
done < "$i"
