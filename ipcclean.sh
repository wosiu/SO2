ipcs -a | grep 0x | awk '{printf( "-Q %s ", $1 )}' | xargs ipcrm
