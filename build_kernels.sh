#!/usr/bin/env bash
platform=$1

dirs=( bin kernel_bin kernel_bin/ioc64 kernel_bin/fpga kernel_bin/fpga/emulator kernel_bin/fpga/device kernel_bin/preprocessed logs/ioc64 logs/fpga logs/fpga/emulator logs/fpga/device )

mkdir -p ${dirs[*]}

function compilecl {
file="$1"
appendeds="$2"
buildopts="$3"
srcpath="device/$file.cl"
binpath=""

echo "Preprocess cl file"
binpath="kernel_bin/preprocessed/${file}_${appendeds}.cl"
cpp $buildopts -P $srcpath -o $binpath
srcpath=$binpath

echo "Compiling cl file $srcpath for platform $platform"
case $platform in
    ioc64)
        binpath="kernel_bin/ioc64/${file}_${appendeds}.ir"
        logpath="logs/ioc64/${file}_${appendeds}.txt"
        ioc64 -cmd=build -input="$srcpath" -ir="$binpath" -output="$logpath" -bo=\""$buildopts"\"
        ;;
    aoc_emulator)
        mkdir -p "kernel_bin/fpga/emulator/${file}_${appendeds}/"
	    binpath="kernel_bin/fpga/emulator/${file}_${appendeds}/${file}_${appendeds}"
        aoc -march=emulator "$srcpath" -o "$binpath" "$buildopts" > "logs/fpga/emulator/${file}_${appendeds}.txt"
        cp "$binpath.aocx" "kernel_bin/fpga/emulator/${file}_${appendeds}.aocx"
	    ;;
    aoc_report)
        mkdir -p "kernel_bin/fpga/device/${file}_${appendeds}/"
	    binpath="kernel_bin/fpga/device/${file}_${appendeds}/${file}_${appendeds}"
        #binpath="kernel_bin/fpga/device/${file}_${appendeds}"
        aoc -rtl "$srcpath" -o "$binpath" "$buildopts" > "logs/fpga/device/${file}_${appendeds}.txt"
        if [ $? -eq 0 ]; then
            #zip -r "reports/${file}_${appendeds}.zip" "$binpath/reports"
            tar -cJf "reports/${file}_${appendeds}.tar.xz" "$binpath/reports"
            cat "$binpath/${file}_${appendeds}.log" | mail -s "${file}_${appendeds} report" -a "reports/${file}_${appendeds}.tar.xz" me@example.com
        fi
        ;;
    aoc_binary_qsub)
        mkdir -p "kernel_bin/fpga/device/${file}_${appendeds}/"
        binpath="kernel_bin/fpga/device/${file}_${appendeds}/${file}_${appendeds}"
        #binpath="kernel_bin/fpga/device/${file}_${appendeds}"
        #~/myqsub-aoc "$srcpath" -o "$binpath" "$buildopts"
        #cp "$binpath.aocx" "kernel_bin/fpga/device/${file}_${appendeds}.aocx"
        qsub <<EOF
#!/bin/bash
#PBS -q skl
#PBS -V
#PBS -j oe
#PBS -l nodes=1:ppn=4
#PBS -l mem=24gb
#PBS -N "build_qsub_aoc_${file}_${appendeds}"
#PBS -m abe
#PBS -M me@example.com

cd "\${PBS_O_WORKDIR}"
cat "" > "logs/fpga/device/${file}_${appendeds}.txt"
aoc "$srcpath" -o "$binpath" "$buildopts" |& tee -a "logs/fpga/device/${file}_${appendeds}.txt"
cp "$binpath.aocx" "kernel_bin/fpga/device/${file}_${appendeds}.aocx"
EOF
        ;;
    aoc_profile_binary_qsub)
        mkdir -p "kernel_bin/fpga/profiling/${file}_${appendeds}/"
        binpath="kernel_bin/fpga/profiling/${file}_${appendeds}/${file}_${appendeds}"
        #binpath="kernel_bin/fpga/device/${file}_${appendeds}"
        #~/myqsub-aoc "$srcpath" -o "$binpath" "$buildopts"
        #cp "$binpath.aocx" "kernel_bin/fpga/device/${file}_${appendeds}.aocx"
        qsub <<EOF
#!/bin/bash
#PBS -q skl
#PBS -V
#PBS -j oe
#PBS -l nodes=1:ppn=4
#PBS -l mem=24gb
#PBS -N "build_qsub_aoc_${file}_${appendeds}"
#PBS -m abe
#PBS -M me@example.com

cd "\${PBS_O_WORKDIR}"
cat "" > "logs/fpga/device/${file}_${appendeds}.txt"
aoc --profile "$srcpath" -o "${binpath}" "$buildopts" |& tee -a "logs/fpga/device/${file}_${appendeds}.txt"
cp "$binpath.aocx" "kernel_bin/fpga/device/${file}_${appendeds}.aocx"
EOF
        ;;
    aoc_binary)
        mkdir -p "kernel_bin/fpga/device/${file}_${appendeds}/"
        binpath="kernel_bin/fpga/device/${file}_${appendeds}/${file}_${appendeds}"
        #binpath="kernel_bin/fpga/device/${file}_${appendeds}"
        aoc "$srcpath" -o "$binpath" "$buildopts" > "logs/fpga/device/${file}_${appendeds}.txt"
        cp "$binpath.aocx" "kernel_bin/fpga/device/${file}_${appendeds}.aocx"
        ;;
    preprocessor)
        #binpath="kernel_bin/preprocessed/${file}_${appendeds}.cl"
        #cpp $buildopts -P $srcpath -o $binpath
        ;;
    *)
        echo "Invalid first argument"
        return -1
        ;;
esac
}


if [[ $# == 4 ]]
then
    compilecl "$2" "$3" "$4"
else


#scsrmv_8 is for FPGAs who cannot do single-cycle 32-bit floating point accumulation
#Comment out if you are using arria 10
    compilecl scsrmv_8 "float_dataunroll16_block2" "-DDATAUNROLL=16 -Dblock_height=2"
    compilecl scsrmv_8 "float_dataunroll16_block4" "-DDATAUNROLL=16 -Dblock_height=4"
    compilecl scsrmv_8 "float_dataunroll16_block8" "-DDATAUNROLL=16 -Dblock_height=8"



    compilecl scsrmv_8 "float_dataunroll8_block2" "-DDATAUNROLL=8 -Dblock_height=2"
    compilecl scsrmv_8 "float_dataunroll8_block4" "-DDATAUNROLL=8 -Dblock_height=4"
    compilecl scsrmv_8 "float_dataunroll8_block8" "-DDATAUNROLL=8 -Dblock_height=8"


    compilecl scsrmv_8 "double_dataunroll8_block2" "-DDATAUNROLL=8 -Dblock_height=2 -DDOUBLE"
    compilecl scsrmv_8 "double_dataunroll8_block4" "-DDATAUNROLL=8 -Dblock_height=4 -DDOUBLE"
    compilecl scsrmv_8 "double_dataunroll8_block8" "-DDATAUNROLL=8 -Dblock_height=8 -DDOUBLE"


    compilecl scsrmv_8 "float_dataunroll24_block8" "-DDATAUNROLL=24 -Dblock_height=8"
    compilecl scsrmv_8 "float_dataunroll8_block16" "-DDATAUNROLL=8 -Dblock_height=16"




    compilecl scsrmv_9 "float_dataunroll16_block2" "-DDATAUNROLL=16 -Dblock_height=2"
    compilecl scsrmv_9 "float_dataunroll16_block4" "-DDATAUNROLL=16 -Dblock_height=4"
    compilecl scsrmv_9 "float_dataunroll16_block8" "-DDATAUNROLL=16 -Dblock_height=8"



    compilecl scsrmv_9 "float_dataunroll8_block2" "-DDATAUNROLL=8 -Dblock_height=2"
    compilecl scsrmv_9 "float_dataunroll8_block4" "-DDATAUNROLL=8 -Dblock_height=4"
    compilecl scsrmv_9 "float_dataunroll8_block8" "-DDATAUNROLL=8 -Dblock_height=8"


    compilecl scsrmv_9 "double_dataunroll8_block2" "-DDATAUNROLL=8 -Dblock_height=2 -DDOUBLE"
    compilecl scsrmv_9 "double_dataunroll8_block4" "-DDATAUNROLL=8 -Dblock_height=4 -DDOUBLE"
    compilecl scsrmv_9 "double_dataunroll8_block8" "-DDATAUNROLL=8 -Dblock_height=8 -DDOUBLE"


    compilecl scsrmv_9 "float_dataunroll24_block8" "-DDATAUNROLL=24 -Dblock_height=8"
    compilecl scsrmv_9 "float_dataunroll8_block16" "-DDATAUNROLL=8 -Dblock_height=16"

fi
exit
