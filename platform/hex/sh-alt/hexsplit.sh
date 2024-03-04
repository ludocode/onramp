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
    output="$2"
    input="$3"
elif [ "$2" = "-o" ]; then
    input="$1"
    output="$3"
else
    echo "Usage: $0 <input_file> -o <output_file>"
    exit 1
fi

# Prepare the output
rm -f "$output" 2>/dev/null
if ! touch "$output"; then
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

            case "$hi" in
                0)
                    case "$lo" in
                        0) printf '\0' >> "$output" ;;
                        1) printf '\1' >> "$output" ;;
                        2) printf '\2' >> "$output" ;;
                        3) printf '\3' >> "$output" ;;
                        4) printf '\4' >> "$output" ;;
                        5) printf '\5' >> "$output" ;;
                        6) printf '\6' >> "$output" ;;
                        7) printf '\7' >> "$output" ;;
                        8) printf '\10' >> "$output" ;;
                        9) printf '\11' >> "$output" ;;
                        A) printf '\12' >> "$output" ;;
                        B) printf '\13' >> "$output" ;;
                        C) printf '\14' >> "$output" ;;
                        D) printf '\15' >> "$output" ;;
                        E) printf '\16' >> "$output" ;;
                        F) printf '\17' >> "$output" ;;
                    esac
                    ;;
                1)
                    case "$lo" in
                        0) printf '\20' >> "$output" ;;
                        1) printf '\21' >> "$output" ;;
                        2) printf '\22' >> "$output" ;;
                        3) printf '\23' >> "$output" ;;
                        4) printf '\24' >> "$output" ;;
                        5) printf '\25' >> "$output" ;;
                        6) printf '\26' >> "$output" ;;
                        7) printf '\27' >> "$output" ;;
                        8) printf '\30' >> "$output" ;;
                        9) printf '\31' >> "$output" ;;
                        A) printf '\32' >> "$output" ;;
                        B) printf '\33' >> "$output" ;;
                        C) printf '\34' >> "$output" ;;
                        D) printf '\35' >> "$output" ;;
                        E) printf '\36' >> "$output" ;;
                        F) printf '\37' >> "$output" ;;
                    esac
                    ;;
                2)
                    case "$lo" in
                        0) printf '\40' >> "$output" ;;
                        1) printf '\41' >> "$output" ;;
                        2) printf '\42' >> "$output" ;;
                        3) printf '\43' >> "$output" ;;
                        4) printf '\44' >> "$output" ;;
                        5) printf '\45' >> "$output" ;;
                        6) printf '\46' >> "$output" ;;
                        7) printf '\47' >> "$output" ;;
                        8) printf '\50' >> "$output" ;;
                        9) printf '\51' >> "$output" ;;
                        A) printf '\52' >> "$output" ;;
                        B) printf '\53' >> "$output" ;;
                        C) printf '\54' >> "$output" ;;
                        D) printf '\55' >> "$output" ;;
                        E) printf '\56' >> "$output" ;;
                        F) printf '\57' >> "$output" ;;
                    esac
                    ;;
                3)
                    case "$lo" in
                        0) printf '\60' >> "$output" ;;
                        1) printf '\61' >> "$output" ;;
                        2) printf '\62' >> "$output" ;;
                        3) printf '\63' >> "$output" ;;
                        4) printf '\64' >> "$output" ;;
                        5) printf '\65' >> "$output" ;;
                        6) printf '\66' >> "$output" ;;
                        7) printf '\67' >> "$output" ;;
                        8) printf '\70' >> "$output" ;;
                        9) printf '\71' >> "$output" ;;
                        A) printf '\72' >> "$output" ;;
                        B) printf '\73' >> "$output" ;;
                        C) printf '\74' >> "$output" ;;
                        D) printf '\75' >> "$output" ;;
                        E) printf '\76' >> "$output" ;;
                        F) printf '\77' >> "$output" ;;
                    esac
                    ;;
                4)
                    case "$lo" in
                        0) printf '\100' >> "$output" ;;
                        1) printf '\101' >> "$output" ;;
                        2) printf '\102' >> "$output" ;;
                        3) printf '\103' >> "$output" ;;
                        4) printf '\104' >> "$output" ;;
                        5) printf '\105' >> "$output" ;;
                        6) printf '\106' >> "$output" ;;
                        7) printf '\107' >> "$output" ;;
                        8) printf '\110' >> "$output" ;;
                        9) printf '\111' >> "$output" ;;
                        A) printf '\112' >> "$output" ;;
                        B) printf '\113' >> "$output" ;;
                        C) printf '\114' >> "$output" ;;
                        D) printf '\115' >> "$output" ;;
                        E) printf '\116' >> "$output" ;;
                        F) printf '\117' >> "$output" ;;
                    esac
                    ;;
                5)
                    case "$lo" in
                        0) printf '\120' >> "$output" ;;
                        1) printf '\121' >> "$output" ;;
                        2) printf '\122' >> "$output" ;;
                        3) printf '\123' >> "$output" ;;
                        4) printf '\124' >> "$output" ;;
                        5) printf '\125' >> "$output" ;;
                        6) printf '\126' >> "$output" ;;
                        7) printf '\127' >> "$output" ;;
                        8) printf '\130' >> "$output" ;;
                        9) printf '\131' >> "$output" ;;
                        A) printf '\132' >> "$output" ;;
                        B) printf '\133' >> "$output" ;;
                        C) printf '\134' >> "$output" ;;
                        D) printf '\135' >> "$output" ;;
                        E) printf '\136' >> "$output" ;;
                        F) printf '\137' >> "$output" ;;
                    esac
                    ;;
                6)
                    case "$lo" in
                        0) printf '\140' >> "$output" ;;
                        1) printf '\141' >> "$output" ;;
                        2) printf '\142' >> "$output" ;;
                        3) printf '\143' >> "$output" ;;
                        4) printf '\144' >> "$output" ;;
                        5) printf '\145' >> "$output" ;;
                        6) printf '\146' >> "$output" ;;
                        7) printf '\147' >> "$output" ;;
                        8) printf '\150' >> "$output" ;;
                        9) printf '\151' >> "$output" ;;
                        A) printf '\152' >> "$output" ;;
                        B) printf '\153' >> "$output" ;;
                        C) printf '\154' >> "$output" ;;
                        D) printf '\155' >> "$output" ;;
                        E) printf '\156' >> "$output" ;;
                        F) printf '\157' >> "$output" ;;
                    esac
                    ;;
                7)
                    case "$lo" in
                        0) printf '\160' >> "$output" ;;
                        1) printf '\161' >> "$output" ;;
                        2) printf '\162' >> "$output" ;;
                        3) printf '\163' >> "$output" ;;
                        4) printf '\164' >> "$output" ;;
                        5) printf '\165' >> "$output" ;;
                        6) printf '\166' >> "$output" ;;
                        7) printf '\167' >> "$output" ;;
                        8) printf '\170' >> "$output" ;;
                        9) printf '\171' >> "$output" ;;
                        A) printf '\172' >> "$output" ;;
                        B) printf '\173' >> "$output" ;;
                        C) printf '\174' >> "$output" ;;
                        D) printf '\175' >> "$output" ;;
                        E) printf '\176' >> "$output" ;;
                        F) printf '\177' >> "$output" ;;
                    esac
                    ;;
                8)
                    case "$lo" in
                        0) printf '\200' >> "$output" ;;
                        1) printf '\201' >> "$output" ;;
                        2) printf '\202' >> "$output" ;;
                        3) printf '\203' >> "$output" ;;
                        4) printf '\204' >> "$output" ;;
                        5) printf '\205' >> "$output" ;;
                        6) printf '\206' >> "$output" ;;
                        7) printf '\207' >> "$output" ;;
                        8) printf '\210' >> "$output" ;;
                        9) printf '\211' >> "$output" ;;
                        A) printf '\212' >> "$output" ;;
                        B) printf '\213' >> "$output" ;;
                        C) printf '\214' >> "$output" ;;
                        D) printf '\215' >> "$output" ;;
                        E) printf '\216' >> "$output" ;;
                        F) printf '\217' >> "$output" ;;
                    esac
                    ;;
                9)
                    case "$lo" in
                        0) printf '\220' >> "$output" ;;
                        1) printf '\221' >> "$output" ;;
                        2) printf '\222' >> "$output" ;;
                        3) printf '\223' >> "$output" ;;
                        4) printf '\224' >> "$output" ;;
                        5) printf '\225' >> "$output" ;;
                        6) printf '\226' >> "$output" ;;
                        7) printf '\227' >> "$output" ;;
                        8) printf '\230' >> "$output" ;;
                        9) printf '\231' >> "$output" ;;
                        A) printf '\232' >> "$output" ;;
                        B) printf '\233' >> "$output" ;;
                        C) printf '\234' >> "$output" ;;
                        D) printf '\235' >> "$output" ;;
                        E) printf '\236' >> "$output" ;;
                        F) printf '\237' >> "$output" ;;
                    esac
                    ;;
                A)
                    case "$lo" in
                        0) printf '\240' >> "$output" ;;
                        1) printf '\241' >> "$output" ;;
                        2) printf '\242' >> "$output" ;;
                        3) printf '\243' >> "$output" ;;
                        4) printf '\244' >> "$output" ;;
                        5) printf '\245' >> "$output" ;;
                        6) printf '\246' >> "$output" ;;
                        7) printf '\247' >> "$output" ;;
                        8) printf '\250' >> "$output" ;;
                        9) printf '\251' >> "$output" ;;
                        A) printf '\252' >> "$output" ;;
                        B) printf '\253' >> "$output" ;;
                        C) printf '\254' >> "$output" ;;
                        D) printf '\255' >> "$output" ;;
                        E) printf '\256' >> "$output" ;;
                        F) printf '\257' >> "$output" ;;
                    esac
                    ;;
                B)
                    case "$lo" in
                        0) printf '\260' >> "$output" ;;
                        1) printf '\261' >> "$output" ;;
                        2) printf '\262' >> "$output" ;;
                        3) printf '\263' >> "$output" ;;
                        4) printf '\264' >> "$output" ;;
                        5) printf '\265' >> "$output" ;;
                        6) printf '\266' >> "$output" ;;
                        7) printf '\267' >> "$output" ;;
                        8) printf '\270' >> "$output" ;;
                        9) printf '\271' >> "$output" ;;
                        A) printf '\272' >> "$output" ;;
                        B) printf '\273' >> "$output" ;;
                        C) printf '\274' >> "$output" ;;
                        D) printf '\275' >> "$output" ;;
                        E) printf '\276' >> "$output" ;;
                        F) printf '\277' >> "$output" ;;
                    esac
                    ;;
                C)
                    case "$lo" in
                        0) printf '\300' >> "$output" ;;
                        1) printf '\301' >> "$output" ;;
                        2) printf '\302' >> "$output" ;;
                        3) printf '\303' >> "$output" ;;
                        4) printf '\304' >> "$output" ;;
                        5) printf '\305' >> "$output" ;;
                        6) printf '\306' >> "$output" ;;
                        7) printf '\307' >> "$output" ;;
                        8) printf '\310' >> "$output" ;;
                        9) printf '\311' >> "$output" ;;
                        A) printf '\312' >> "$output" ;;
                        B) printf '\313' >> "$output" ;;
                        C) printf '\314' >> "$output" ;;
                        D) printf '\315' >> "$output" ;;
                        E) printf '\316' >> "$output" ;;
                        F) printf '\317' >> "$output" ;;
                    esac
                    ;;
                D)
                    case "$lo" in
                        0) printf '\320' >> "$output" ;;
                        1) printf '\321' >> "$output" ;;
                        2) printf '\322' >> "$output" ;;
                        3) printf '\323' >> "$output" ;;
                        4) printf '\324' >> "$output" ;;
                        5) printf '\325' >> "$output" ;;
                        6) printf '\326' >> "$output" ;;
                        7) printf '\327' >> "$output" ;;
                        8) printf '\330' >> "$output" ;;
                        9) printf '\331' >> "$output" ;;
                        A) printf '\332' >> "$output" ;;
                        B) printf '\333' >> "$output" ;;
                        C) printf '\334' >> "$output" ;;
                        D) printf '\335' >> "$output" ;;
                        E) printf '\336' >> "$output" ;;
                        F) printf '\337' >> "$output" ;;
                    esac
                    ;;
                E)
                    case "$lo" in
                        0) printf '\340' >> "$output" ;;
                        1) printf '\341' >> "$output" ;;
                        2) printf '\342' >> "$output" ;;
                        3) printf '\343' >> "$output" ;;
                        4) printf '\344' >> "$output" ;;
                        5) printf '\345' >> "$output" ;;
                        6) printf '\346' >> "$output" ;;
                        7) printf '\347' >> "$output" ;;
                        8) printf '\350' >> "$output" ;;
                        9) printf '\351' >> "$output" ;;
                        A) printf '\352' >> "$output" ;;
                        B) printf '\353' >> "$output" ;;
                        C) printf '\354' >> "$output" ;;
                        D) printf '\355' >> "$output" ;;
                        E) printf '\356' >> "$output" ;;
                        F) printf '\357' >> "$output" ;;
                    esac
                    ;;
                F)
                    case "$lo" in
                        0) printf '\360' >> "$output" ;;
                        1) printf '\361' >> "$output" ;;
                        2) printf '\362' >> "$output" ;;
                        3) printf '\363' >> "$output" ;;
                        4) printf '\364' >> "$output" ;;
                        5) printf '\365' >> "$output" ;;
                        6) printf '\366' >> "$output" ;;
                        7) printf '\367' >> "$output" ;;
                        8) printf '\370' >> "$output" ;;
                        9) printf '\371' >> "$output" ;;
                        A) printf '\372' >> "$output" ;;
                        B) printf '\373' >> "$output" ;;
                        C) printf '\374' >> "$output" ;;
                        D) printf '\375' >> "$output" ;;
                        E) printf '\376' >> "$output" ;;
                        F) printf '\377' >> "$output" ;;
                    esac
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
done < "$input"
