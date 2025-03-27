cwd=$(pwd)
dir_temp="$(dirname -- $(readlink -fn -- "$0"; echo x))"
dir="${dir_temp%x}"
cd $dir
qmake VieSchedppGUI.pro -o ./build
make
cd $cwd