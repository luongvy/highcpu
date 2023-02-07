cmd_/root/test/hello/lkm_hello1.ko := ld -r -m elf_x86_64 -T ./scripts/module-common.lds --build-id  -o /root/test/hello/lkm_hello1.ko /root/test/hello/lkm_hello1.o /root/test/hello/lkm_hello1.mod.o
