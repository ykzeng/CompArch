echo "=========================================================="
echo "=========================LRU Results======================"
echo "=========================================================="
export DAN_POLICY=0
#./efectiu traces/400.perlbench-50B.trace.gz
#./efectiu traces/482.sphinx3-417B.trace.gz
./efectiu traces/401.bzip2-226B.trace.gz

echo "=========================================================="
echo "========================Random Results===================="
echo "=========================================================="
export DAN_POLICY=1
#./efectiu traces/482.sphinx3-417B.trace.gz
#./efectiu traces/400.perlbench-50B.trace.gz
./efectiu traces/401.bzip2-226B.trace.gz

echo "=========================================================="
echo "======================MyReplace Results==================="
echo "=========================================================="
export DAN_POLICY=2
#./efectiu traces/482.sphinx3-417B.trace.gz
#./efectiu traces/400.perlbench-50B.trace.gz
./efectiu traces/401.bzip2-226B.trace.gz

echo "=========================================================="
echo "=========================NRU Results======================"
echo "=========================================================="
export DAN_POLICY=3
#./efectiu traces/482.sphinx3-417B.trace.gz
#./efectiu traces/400.perlbench-50B.trace.gz
./efectiu traces/401.bzip2-226B.trace.gz

echo "=========================================================="
echo "=======================HP RRIP Results===================="
echo "=========================================================="
export DAN_POLICY=4
#./efectiu traces/482.sphinx3-417B.trace.gz
#./efectiu traces/400.perlbench-50B.trace.gz
./efectiu traces/401.bzip2-226B.trace.gz

echo "=========================================================="
echo "=======================FP RRIP Results===================="
echo "=========================================================="
export DAN_POLICY=5
#./efectiu traces/482.sphinx3-417B.trace.gz
#./efectiu traces/400.perlbench-50B.trace.gz
./efectiu traces/401.bzip2-226B.trace.gz
