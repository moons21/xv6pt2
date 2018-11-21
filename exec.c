#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

int
exec(char *path, char **argv)
{
  //cprintf("\%-------------EXEC-------------%\n");
  char *s, *last;
  int i, off;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  uint stack;
  uint stackSize;
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;
  struct proc *curproc = myproc();

  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    cprintf("exec: fail\n");
    return -1;
  }
  ilock(ip);
  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;

  // Load program into memory.
  sz = 0;
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;

  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  sz = PGROUNDUP(sz); // heap pointer

  if((sz = allocuvm(pgdir, sz, sz + 1*PGSIZE)) == 0)
    goto bad;
  
  // NEW
  stack = KERNBASE - 4;	// Bottom of stack (starts below kernel)
  stack = PGROUNDDOWN(stack); // round it down tho
  if((stack = allocuvm(pgdir, stack - 1*PGSIZE, stack)) == 0)
    panic("Couldnt allocate stack!\n");
  stackSize = 1; // size is 1

  // Buffer stuff FIXME
  clearpteu(pgdir, (char*)(sz - 1*PGSIZE)); // OLD FIXME
  //clearpteu(pgdir, (char*)(stackBot - 2*PGSIZE));

  // UPDATE STACK POINTER:
  //sp = sz; // OLD FIXME
  sp = stack; // Pointer is at bottom of stack

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));

  // Commit to the user image.
  oldpgdir = curproc->pgdir;
  curproc->pgdir = pgdir;

  curproc->sz = sz; 
  curproc->stackBot = stack; // set to bottom of stack (TOP OF UESRSPACE)
  curproc->stackSize = stackSize; 

  curproc->tf->eip = elf.entry;  // main

  curproc->tf->esp = sp;
  /*
  cprintf("-----------------------------\n");
  cprintf("Kernbase: %d\n", KERNBASE -4);
  cprintf("PID: %d\n", curproc->pid);
  cprintf("heap: %d\n", curproc->heap);
  cprintf("Stack: %d\n", curproc->sz);
  cprintf("StackSize: %d\n", curproc->stackSize);
  cprintf("-----------------------------\n");
  */
  switchuvm(curproc);
  freevm(oldpgdir);
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}
