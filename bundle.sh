rm -rf brawl.app
mkdir brawl.app
mkdir -p brawl.app/bin
mkdir -p brawl.app/lib
cp -R res brawl.app
ldd build/brawl | awk '{print $3}' | xargs -I {} cp {} brawl.app/lib/
rm brawl.app/lib/libc.so.*
rm brawl.app/lib/libm.so.*
cp -R build/brawl brawl.app/bin/
echo 'PTH=$(dirname $(readlink -f $0))' > brawl.app/brawl
echo "LD_LIBRARY_PATH=\$PTH/lib \$PTH/bin/brawl -r \$PTH/res" >> brawl.app/brawl
chmod +x brawl.app/brawl
tar czf brawl.app.tar.gz brawl.app
