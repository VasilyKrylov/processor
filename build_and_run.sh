clear
cd asm
./asm my_asm/$1.my_asm
cp out.spu ../spu/out.spu
cd ../spu
./spu out.spu