#!/bin/bash

REF=.ref
RES=.res
OUT=.out
exec=./../build/42sh

file=false
str=false
redir=false
nothing=false

step1=false
step2=false
step3=false
step4=false

specific=""

if [ "$#" -ge 1 ]; then
    if [ "$1" = "--help" ]; then
	echo "Usage: ./tests/test.sh [ file | str | redir ] [ [ 1 | 2 | 3 | 4 ] | [ specific ] ]"
	exit 0
    elif [ "$1" = "file" ]; then
	file=true
    elif [ "$1" = "str" ]; then
	str=true
    elif [ "$1" = "redir" ]; then
	redir=true
    elif [ "$1" = "1" ]; then
	step1=true
    elif [ "$1" = "2" ]; then
	step2=true
    elif [ "$1" = "3" ]; then
	step3=true
    elif [ "$1" = "4" ]; then
	step4=true
    else
	specific="$1"
    fi

    if [ "$#" -ge 2 ]; then
        if [ "$2" = "1" ]; then
	    step1=true
        elif [ "$2" = "2" ]; then
	    step2=true
        elif [ "$2" = "3" ]; then
	    step3=true
        elif [ "$2" = "4" ]; then
	    step4=true
	else
	    specific="$2"
        fi
    fi
fi

if [ $file = false ]  && [ $str = false ] && [ $redir = false ]; then
    nothing=true
fi

if [ $step1 = false ] && [ $step2 = false ] && [ $step3 = false ] && [ $step4 = false ]; then
    step1=true
    step2=true
    step3=true
    step4=true
fi

cd tests

setup() {
    echo "=================== Compiling ==================="
    cd ../
    meson setup build
    meson configure -Db_sanitize=address build
    ninja -C build
    cd tests/
    echo -e "\n"
}

cleanup() {
    echo -e "\n"
    echo "================== Cleaning up =================="
   # rm -rd build
    rm ../log_file
    rm log_file
    rm .ref
    rm .res
    rm .out
}

check_res()
{
    if [ ! -s "$RES" ] && [ $code -eq 0 ]; then
        echo -e "\e[1;32m OK!\e[0m"
    else
        echo -e "\e[1;31m ERROR!\e[0m"
        echo "============== TEST ==============="
        cat "$1"
        echo -e "\n=============== REF ==============="
        cat "$REF"
        echo -e "\n=============== OUT ==============="
        cat "$OUT"
        echo -e "\nexit code: $code"
        echo -e "=========== PRETTY PRINT =========="
        "$exec" --pretty-print "$1"
        echo -e "===================================\n"
    fi
}

test_str() {
    content=$(cat $1)
    "$exec" -c "$content" > "$OUT"
    code="$?"

    cat "$2" > "$REF"

    diff "$REF" "$OUT" > "$RES"
    check_res $1
}

test_file()
{
    "$exec" "$1" > "$OUT"
    code="$?"

    cat "$2" > "$REF"

    diff "$REF" "$OUT" > "$RES"
    check_res $1
}

test_redir()
{
    ("$exec" < "$1" ) > "$OUT"
    code="$?"

    cat "$2" > "$REF"

    diff "$REF" "$OUT" > "$RES"

    check_res $1
}

msg_str="========== ./42sh -c \"string\" =========="
msg_redir="========== ./42sh < \"filename.sh\" =========="
msg_file="========== ./42sh \"filename.sh\" =========="

main_aux()
{
    input=$(ls -R "$1"/*/*.in | tr '\n' ' ')

    $file && echo -e "$msg_file"
    $str && echo -e "$msg_str"
    $redir && echo -e "$msg_redir"

    last_dir=""
    for file_in in $input; do
        dir=$(echo "$file_in" | cut -d '/' -f2)
        if [ "$last_dir" != "$dir" ]; then
            last_dir="$dir"
            echo -e "\n======> $dir"
        fi
        file_out=$(echo "$file_in" | sed 's/\(.\.\)in/\1out/g')
        echo -n "==> $file_in" | sed "s/$1\/\(.\)/\1/g"
        $file && test_file "$file_in" "$file_out"
        $str && test_str "$file_in" "$file_out"
        $redir && test_redir "$file_in" "$file_out"
    done

    echo ""
}

main_nothing()
{
    if [ $step1 = true ]; then
        echo -e "\n===================> STEP 1 <====================\n"
        file=true
        $step1 && main_aux "step1"
        file=false
        str=true
        $step1 && main_aux "step1"
        str=false
        redir=true
        $step1 && main_aux "step1"
        redir=false
    fi
    if [ $step2 = true ]; then
        echo -e "\n===================> STEP 2 <====================\n"
        file=true
        $step2 && main_aux "step2"
        file=false
        str=true
        $step2 && main_aux "step2"
        str=false
        redir=true
        $step2 && main_aux "step2"
        redir=false
    fi
    if [ $step3 = true ]; then
        echo -e "\n===================> STEP 3 <====================\n"
        file=true
        $step3 && main_aux "step3"
        file=false
        str=true
        $step3 && main_aux "step3"
        str=false
        redir=true
        $step3 && main_aux "step3"
        redir=false
    fi
    if [ $step4 = true ]; then
        echo -e "\n===================> STEP 4 <====================\n"
        file=true
        $step4 && main_aux "step4"
        file=false
        str=true
        $step4 && main_aux "step4"
        str=false
        redir=true
        $step4 && main_aux "step4"
        redir=false
    fi
}

main_specific()
{
    input=$(ls -d */*/*.in | grep "$1" | tr '\n' ' ')

    echo -e "\n======> $1"

    if [ $file = true ]; then
	echo -e "$msg_file"

	for file_in in $input; do
	    file_out=$(echo "$file_in" | sed 's/\(.\.\)in/\1out/g')
	    echo -n "==> $file_in" | sed "s/.\/\(.\.in\)/\1/g"

	    test_file "$file_in" "$file_out"
	done
	echo ""
    fi

    if [ $str = true ]; then
	echo -e "$msg_str"

	for file_in in $input; do
	    file_out=$(echo "$file_in" | sed 's/\(.\.\)in/\1out/g')
	    echo -n "==> $file_in" | sed "s/.\/\(.\.in\)/\1/g"

	    test_str "$file_in" "$file_out"
	done
	echo ""
    fi

    if [ $redir = true ]; then
	echo -e "$msg_redir"

	for file_in in $input; do
	    file_out=$(echo "$file_in" | sed 's/\(.\.\)in/\1out/g')
	    echo -n "==> $file_in" | sed "s/.\/\(.\.in\)/\1/g"

	    test_redir "$file_in" "$file_out"
	done
    fi
}

main()
{
    setup
    if [ "$specific" != "" ]; then
	if [ $nothing = true ]; then
	    file=true
	    str=true
	    redir=true
	fi
	main_specific "$specific"
	cleanup
	exit
    fi
    if [ $nothing = true ]; then
	main_nothing
	cleanup
	exit
    fi
    $step1 && echo -e "\n===================> STEP 1 <====================\n" && main_aux "step1"
    $step2 && echo -e "\n===================> STEP 2 <====================\n" && main_aux "step2"
    $step3 && echo -e "\n===================> STEP 3 <====================\n" && main_aux "step3"
    $step4 && echo -e "\n===================> STEP 4 <====================\n" && main_aux "step4"
    cleanup
}

main
