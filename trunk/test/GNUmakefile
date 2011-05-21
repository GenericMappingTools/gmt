tests test:
	run_gmt_tests.sh */*.sh

failed:
	run_gmt_tests.sh `cut -d: -f1 fail_count.d`

clean spotless:
	rm -f */*.ps */*.png
