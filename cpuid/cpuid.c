/* Intel and AMD x86 CPUID display program v 3.3 (1 Jan 2002)
 * Copyright 2002 Phil Karn, KA9Q
 * Updated 24 Apr 2001 to latest Intel CPUID spec
 * Updated 22 Dec 2001 to decode Intel flag 28, hyper threading
 * Updated 1 Jan 2002 to cover AMD Duron, Athlon
 * Updated 24 Aug 2009 to decode additional Intel flags
 * Updated 23 May 2010 to decode additional Intel flags
 * May be used under the terms of the GNU Public License (GPL)

 * Reference documents:
 * http://www.intel.com/Assets/PDF/appnote/241618.pdf (AN-485 August 2009)
 * http://developer.intel.com/design/Pentium4/manuals/24547103.pdf
 * http://developer.intel.com/design/pentiumiii/applnots/24512501.pdf (AP-909)
 * http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/20734.pdf
 * http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/24594.pdf
 * http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
 *
 */

#include <stdio.h>
#include <stdlib.h>

void decode_intel_tlb(int);
void decode_cyrix_tlb(int);
void dointel(int),doamd(int),docyrix(int);
void printregs(int eax,int ebx,int ecx,int edx);

#define MAXBRANDS 24
char *Brands[MAXBRANDS] = {
  NULL,
  "Celeron processor",
  "Pentium III processor",
  "Intel Pentium III Xeon processor",
  "Intel Pentium III processor",
  NULL,
  "Mobile Intel Pentium III processor-M",
  "Mobile Intel Celeron processor",
  /* 8 */
  "Intel Pentium 4 processor",
  "Intel Pentium 4 processor",
  "Intel Celeron processor",
  "Intel Xeon processor",
  "Intel Xeon processor MP",
  NULL,
  "Mobile Intel Pentium 4 processor-M",
  "Mobile Intel Celeron processor",
  /* 16 */
  NULL,
  "Mobile Genuine Intel processor",
  "Intel Celeron M processor",
  "Mobile Intel Celeron processor",
  "Intel Celeron processor",
  "Mobile Genuine Intel processor",
  "Intel Pentium M processor",
  "Mobile Intel Celeron processor",
};

#define cpuid(in,a,b,c,d)\
  asm("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));
#define cpuidx(in,in2,a,b,c,d)\
  asm("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in), "c" (in2));

int main(){
  int i;
  unsigned long li,maxi,maxei,ebx,ecx,edx,unused;

  /* Insert code here to test if CPUID instruction is available */

  /* Dump all the CPUID results in raw hex */
  cpuid(0,maxi,unused,unused,unused);
  maxi &= 0xffff; /* The high-order word is non-zero on some Cyrix CPUs */
  printf(" eax in    eax      ebx      ecx      edx\n");
  for(i=0;i<=maxi;i++){
    unsigned long eax,ebx,ecx,edx;
    ecx = 0; /* basic query for leaves that dependent on it */

    cpuidx(i,ecx,eax,ebx,ecx,edx);
    printf("%08x %08lx %08lx %08lx %08lx\n",i,eax,ebx,ecx,edx);
  }
  cpuid(0x80000000,maxei,unused,unused,unused);
  for(li=0x80000000;li<=maxei;li++){
    unsigned long eax,ebx,ecx,edx;

    cpuid(li,eax,ebx,ecx,edx);
    printf("%08lx %08lx %08lx %08lx %08lx\n",li,eax,ebx,ecx,edx);
  }
  printf("\n");

  /* Vendor ID and max CPUID level supported */
  cpuid(0,unused,ebx,ecx,edx);
  printf("Vendor ID: \"");
  for(i=0;i<4;i++)
    putchar(ebx >> (8*i));
  for(i=0;i<4;i++)
    putchar(edx >> (8*i));
  for(i=0;i<4;i++)
    putchar(ecx >> (8*i));
  printf("\"; CPUID level %ld\n\n",maxi);

  switch(ebx){
  case 0x756e6547: /* Intel */
    dointel(maxi);
    break;
  case 0x68747541: /* AMD */
    doamd(maxi);
    break;
  case 0x69727943: /* Cyrix */
    docyrix(maxi);
    break;
  default:
    printf("Unknown vendor\n");
    break;
  }
  exit(0);
}

char *Intel_feature_flags1[32] = {
  "FPU    Floating Point Unit",
  "VME    Virtual 8086 Mode Enhancements",
  "DE     Debugging Extensions",
  "PSE    Page Size Extensions",
  "TSC    Time Stamp Counter",
  "MSR    Model Specific Registers",
  "PAE    Physical Address Extension",
  "MCE    Machine Check Exception",
  "CX8    COMPXCHG8B Instruction",
  "APIC   On-chip Advanced Programmable Interrupt Controller present and enabled",
  "10     Reserved",
  "SEP    Fast System Call",
  "MTRR   Memory Type Range Registers",
  "PGE    PTE Global Flag",
  "MCA    Machine Check Architecture",
  "CMOV   Conditional Move and Compare Instructions",
  "FGPAT  Page Attribute Table",
  "PSE-36 36-bit Page Size Extension",
  "PSN    Processor Serial Number present and enabled",
  "CLFSH  CFLUSH instruction",
  "20     reserved",
  "DS     Debug store",
  "ACPI   Thermal Monitor and Clock Ctrl",
  "MMX    MMX instruction set",
  "FXSR   Fast FP/MMX Streaming SIMD Extensions save/restore",
  "SSE    Streaming SIMD Extensions instruction set",
  "SSE2   SSE2 extensions",
  "SS     Self Snoop",
  "HT     Hyper Threading",
  "TM     Thermal monitor",
  "30     reserved",
  "PBE    Pending Break Enable"
};

char *Intel_feature_flags2[32] = {
  "SSE3     SSE3 extensions",
  "PCLMULDQ PCLMULDQ instruction",
  "DTES64   64-bit debug store",
  "MONITOR  MONITOR/MWAIT instructions",
  "DS-CPL   CPL Qualified Debug Store",
  "VMX      Virtual Machine Extensions",
  "SMX      Safer Mode Extension",
  "EST      Enhanced Intel SpeedStep Technology",
  "TM2      Thermal Monitor 2",
  "SSSE3    Supplemental Streaming SIMD Extension 3",
  "CNXT-ID  L1 Context ID",
  "11       reserved11",
  "FMA      Fused Multiply-Add",
  "CX16     CMPXCHG16B",
  "xTPR     Send Task Priority messages",
  "PDCM     Perfmon and debug capability",
  NULL,
  "PCID     Process-Context Identifiers",
  "DCA      Direct Cache Access",
  "SSE4.1   Streaming SIMD Extension 4.1",
  "SSE4.2   Streaming SIMD Extension 4.2",
  "x2APIC   Extended xAPIC support",
  "MOVBE    MOVBE instruction",
  "POPCNT   POPCNT instruction",
  "TSC-Deadline",
  "AESNI    AES Instruction set",
  "XSAVE    XSAVE/XSTOR states",
  "OSXSAVE  OS-enabled extended state managerment",
  "AVX      AVX extensions",
  "F16C     16-bit floating convert instructions",
  "RDRAND   RDRAND instruction",
  NULL
};

char *Intel_ext_feature_flags1[32] = {
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL,
  "SYSCALL   SYSCALL/SYSRET instructions",
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL,
  "XD-bit    Execution Disable bit",
  NULL, NULL, NULL,
  NULL, NULL,
  "1GBP      1 GByte pages are available",
  "RDTSCP    RDTSCP and IA32_TSC_AUX are available",
  NULL,
  "EM64T     Intel Extended Memory 64 Technology",
  NULL, NULL
};

char *Intel_ext_feature_flags2[32] = {
  "LAHF      LAHF/SAHF available in IA-32e mode",
  NULL, NULL, NULL, NULL,
  "LZCNT",
  NULL, NULL,
  "PREFETCHW",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

char *Intel_struct_ext_feature_flags_7_0_b[32] = {
  "FSGSBASE", "IA32_TSC_ADJUST MSR",
  NULL, "BMI1", "HLE", "AVX2", NULL, "SMEP",
  "BMI2", "Enhanced REP MOVSB/STOSB",
  "INVPCID", "RTM", "Quality of Service Monitoring capability",
  "Deprecates FPU CS and FPU DS", NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

/* Intel-specific information */
void dointel(int maxi){
  printf("Intel-specific functions:\n");

  if(maxi >= 1){
    /* Family/model/type etc */
    int clf,apic_id,feature_flags;
    int feature_flags2 = 0;
    int ext_feature_flags = 0;
    int ext_feature_flags2 = 0;
    int basic_family, extended_family, basic_model, extended_model;
    int family, model;
    unsigned long eax,ebx,ecx,edx;
    int stepping,type,reserved,brand,siblings;
    int i;

    cpuid(1,eax,ebx,ecx,edx);
    printf("Version %08lx:\n",eax);
    stepping = eax & 0xf;
    basic_family = (eax >> 8) & 0xf;
    extended_family = (eax >> 20) & 0xff;
    family = basic_family + extended_family;
    basic_model = (eax >> 4) & 0xf;
    extended_model = (eax >> 16) & 0xf;
    model = (extended_model << 4) | basic_model;
    type = (eax >> 12) & 0x3;
    reserved = eax & 0xf000c000l;
    clf = (ebx >> 8) & 0xff;
    apic_id = (ebx >> 24) & 0xff;
    siblings = (ebx >> 16) & 0xff;
    feature_flags = edx;
    feature_flags2 = ecx;

    printf("Type %d - ",type);
    switch(type){
    case 0:
      printf("Original OEM");
      break;
    case 1:
      printf("Overdrive");
      break;
    case 2:
      printf("Dual-capable");
      break;
    case 3:
      printf("Reserved");
      break;
    }
    printf("\n");

    printf("Family %d - ",family);
    switch(family){
    case 3:
      printf("i386");
      break;
    case 4:
      printf("i486");
      break;
    case 5:
      printf("Pentium");
      break;
    case 6:
      printf("Pentium Pro");
      break;
    case 15:
      printf("Pentium 4");
    }
    printf("\n");
    printf("Model %d - ",model);
    switch(family){
    case 3:
      break;
    case 4:
      switch(model){
      case 0:
      case 1:
	printf("DX");
	break;
      case 2:
	printf("SX");
	break;
      case 3:
	printf("487/DX2");
	break;
      case 4:
	printf("SL");
	break;
      case 5:
	printf("SX2");
	break;
      case 7:
	printf("write-back enhanced DX2");
	break;
      case 8:
	printf("DX4");
	break;
      }
      break;
    case 5:
      switch(model){
      case 1:
	printf("60/66");
	break;
      case 2:
	printf("75-200");
	break;
      case 3:
	printf("for 486 system");
	break;
      case 4:
	printf("MMX");
	break;
      }
      break;
    case 6:
      switch(model){
      case 1:
	printf("Pentium Pro");
	break;
      case 3:
	printf("Pentium II Model 3");
	break;
      case 5:
	printf("Pentium II Model 5/Xeon/Celeron");
	break;
      case 6:
	printf("Celeron");
	break;
      case 7:
	printf("Pentium III/Pentium III Xeon - external L2 cache");
	break;
      case 8:
	printf("Pentium III/Pentium III Xeon - internal L2 cache");
	break;
      case 9:
	printf("Intel Pentium M processor model 9");
	break;
      case 10:
	printf("Pentium III Xeon processor model A");
	break;
      case 11:
	printf("Intel Pentium III processor model B");
	break;
      case 13:
	printf("Intel Pentium M processor model D");
	break;
      case 14:
	printf("Intel Core family processor, 65nm");
	break;
      case 15:
	printf("Intel Core2 family processor, 65nm");
	break;
      case 21:
	printf("Intel EP80579 integrated processor");
	break;
      case 22:
	printf("Intel Celeron processor model 16h, 65nm");
	break;
      case 23:
	printf("Intel Core2 Extreme or Xeon processor, 45nm");
	break;
      case 28:
	printf("Intel Atom processor, 45nm");
	break;
      case 30:
	printf("Intel Corei7 or Xeon processor, 45nm");
	break;
      case 31:
	printf("Intel Xeon processor MP, 45nm");
	break;
      }
      break;
    case 15:
      printf("Intel Pentium 4 processor (generic) or newer");
      break;
    }
    printf("\n");
    printf("Stepping %d\n",stepping);

    printf("Reserved %x\n\n",reserved);

    brand = ebx & 0xff;
    if(brand > 0){
      printf("Brand index: %d [",brand);
      if(brand  < MAXBRANDS && Brands[brand] != NULL){
	printf("%s]\n",Brands[brand]);
      } else {
	printf("not in table]\n");
      }
    }
    cpuid(0x80000000,eax,ebx,ecx,edx);
    if(eax & 0x80000000){
      /* Extended feature/signature bits supported */
      int maxe = eax;
      if (maxe >= 0x80000001) {
	unsigned long eax,ebx,ecx,edx;
	cpuid(0x80000001,eax,ebx,ecx,edx);
	ext_feature_flags = edx;
	ext_feature_flags2 = ecx;
      }
      if(maxe >= 0x80000004){
	int i;

	printf("Extended brand string: \"");
	for(i=0x80000002;i<=0x80000004;i++){
	  unsigned long eax,ebx,ecx,edx;

	  cpuid(i,eax,ebx,ecx,edx);
	  printregs(eax,ebx,ecx,edx);
	}
	printf("\"\n");
      }
    }
    if(clf)
      printf("CLFLUSH instruction cache line size: %d\n",clf);

    if(apic_id)
      printf("Initial APIC ID: %d\n",apic_id);

    if(feature_flags & (1<<28)){
      printf("Hyper threading siblings: %d\n",siblings);
    }

    printf("\nFeature flags set 1 (CPUID.01H:EDX): %08x:\n",
      feature_flags);
    for(i=0;i<32;i++){
      if(feature_flags & (1<<i)){
	printf("%s\n",Intel_feature_flags1[i]);
      }
    }
    if(feature_flags2) {
      printf("\nFeature flags set 2 (CPUID.01H:ECX): %08x:\n",
	feature_flags2);
      for (i = 0; i < 32; ++i) {
        if (feature_flags2 & (1 << i)) {
          const char* fn = Intel_feature_flags2[i];
          if (fn != NULL)
            printf("%s\n", fn);
          else
            printf("%d - unknown feature\n", i);
        }
      }
    }
    if(ext_feature_flags) {
      printf("\nExtended feature flags set 1 (CPUID.80000001H:EDX): %08x\n",
	ext_feature_flags);
      for (i = 0; i < 32; ++i) {
        if (ext_feature_flags & (1 << i)) {
          const char* fn = Intel_ext_feature_flags1[i];
          if (fn != NULL)
            printf("%s\n", fn);
          else
            printf("%d - unknown feature\n", i);
        }
      }
    }
    if(ext_feature_flags2) {
      printf("\nExtended feature flags set 2 (CPUID.80000001H:ECX): %08x\n",
	ext_feature_flags2);
      for (i = 0; i < 32; ++i) {
        if (ext_feature_flags2 & (1 << i)) {
          const char* fn = Intel_ext_feature_flags2[i];
          if (fn != NULL)
            printf("%s\n", fn);
          else
            printf("%d - unknown feature\n", i);
        }
      }
    }
    printf("\n");
  }
  if(maxi >= 2){
    /* Decode TLB and cache info */
    int ntlb,i;

    ntlb = 255;
    printf("Old-styled TLB and cache info:\n");
    for(i=0;i<ntlb;i++){
      unsigned long eax,ebx,ecx,edx;

      cpuid(2,eax,ebx,ecx,edx);
      ntlb =  eax & 0xff;
      decode_intel_tlb(eax >> 8);
      decode_intel_tlb(eax >> 16);
      decode_intel_tlb(eax >> 24);

      if((ebx & 0x80000000) == 0){
	decode_intel_tlb(ebx);
	decode_intel_tlb(ebx >> 8);
	decode_intel_tlb(ebx >> 16);
	decode_intel_tlb(ebx >> 24);
      }
      if((ecx & 0x80000000) == 0){
	decode_intel_tlb(ecx);
	decode_intel_tlb(ecx >> 8);
	decode_intel_tlb(ecx >> 16);
	decode_intel_tlb(ecx >> 24);
      }
      if((edx & 0x80000000) == 0){
	decode_intel_tlb(edx);
	decode_intel_tlb(edx >> 8);
	decode_intel_tlb(edx >> 16);
	decode_intel_tlb(edx >> 24);
      }
    }
  }
  if(maxi >= 3){
    /* Pentium III CPU serial number */
    unsigned long signature,unused,ecx,edx;

    cpuid(1,signature,unused,unused,unused);
    cpuid(3,unused,unused,ecx,edx);
    printf("\nProcessor serial: ");
    printf("%04lX",signature >> 16);
    printf("-%04lX",signature & 0xffff);
    printf("-%04lX",edx >> 16);
    printf("-%04lX",edx & 0xffff);
    printf("-%04lX",ecx >> 16);
    printf("-%04lX\n",ecx & 0xffff);
  }
  if (maxi >= 4) {
    unsigned long index, eax, ebx, ecx, edx, ctype;
    unsigned long linesize, partitions, ways, sets, t;
    for (index = 0; ; ++index) {
      cpuidx(4,index,eax,ebx,ecx,edx);
      ctype = eax&31;
      if (ctype == 0)
	break;
      if (index==0)
	printf("\nDeterministic Cache Parameters:\n");
      printf("index=%lu: eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx\n> ",
	index, eax, ebx, ecx, edx);
      switch (ctype) {
	case 1:
	  printf("Data cache");
	  break;
	case 2:
	  printf("Instruction cache");
	  break;
	case 3:
	  printf("Unified cache");
	  break;
	default:
	  printf("Unknown cache type (%lu)", ctype);
      }
      printf(", level %lu", (eax>>5)&7);
      if (eax&0x100u)
	printf(", self initializing");
      if (eax&0x200u)
	printf(", fully associative");
      printf("\n");
      sets = ecx+1;
      ways = ((ebx>>22)&0x3ffu)+1;
      partitions = ((ebx>>12)&0x3ffu)+1;
      linesize = (ebx&0xfffu)+1;
      printf("> %lu sets", sets);
      printf(", %lu ways", ways);
      printf(", %lu partitions", partitions);
      printf(", line size %lu", linesize);
      printf("\n> full size %lu bytes\n", sets*ways*partitions*linesize);
      t = (eax>>14)&0xfffu;
      if (t>0)
	printf("> shared between up to %lu threads\n", t+1);
      t = (eax>>26)&0x3fu;
      if (index==0 && t>0)
	printf("> NB this package has up to %lx threads\n", t+1);
    }
  }
  if (maxi >= 7) {
    unsigned i;
    unsigned long eax, ebx, ecx, edx;
    cpuidx(7, 0, eax, ebx, ecx, edx);
    if (ebx != 0) {
      printf("\nStructured feature flags CPUID.07H.00H:EBX: %08lx:\n",
	ebx);
      for(i=0;i<32;i++){
	if(ebx & (1<<i)){
	  const char* fn = Intel_struct_ext_feature_flags_7_0_b[i];
	  if (fn != NULL)
	    printf("%s\n", fn);
	  else
	    printf("%d - unknown feature\n", i);
	}
      }
    }
  }
}
void printregs(int eax,int ebx,int ecx,int edx){
  int j;
  char string[17];

  string[16] = '\0';
  for(j=0;j<4;j++){
    string[j] = eax >> (8*j);
    string[j+4] = ebx >> (8*j);
    string[j+8] = ecx >> (8*j);
    string[j+12] = edx >> (8*j);
  }
  printf("%s",string);
}


/* Decode Intel TLB and cache info descriptors */
void decode_intel_tlb(int x){
  x &= 0xff;
  if(x != 0)
    printf("%02x: ",x);
  switch(x){
  case 0:
    break;
  case 0x1:
    printf("Instruction TLB: 4KB pages, 4-way set assoc, 32 entries\n");
    break;
  case 0x2:
    printf("Instruction TLB: 4MB pages, 4-way set assoc, 2 entries\n");
    break;
  case 0x3:
    printf("Data TLB: 4KB pages, 4-way set assoc, 64 entries\n");
    break;
  case 0x4:
    printf("Data TLB: 4MB pages, 4-way set assoc, 8 entries\n");
    break;
  case 0x5:
    printf("Data TLB: 4MB pages, 4-way set assoc, 32 entries\n");
    break;
  case 0x6:
    printf("1st-level instruction cache: 8KB, 4-way set assoc, 32 byte line size\n");
    break;
  case 0x8:
    printf("1st-level instruction cache: 16KB, 4-way set assoc, 32 byte line size\n");
    break;
  case 0x9:
    printf("1st-level instruction cache: 32KB, 4-way set assoc, 64 byte line size\n");
    break;
  case 0xa:
    printf("1st-level data cache: 8KB, 2-way set assoc, 32 byte line size\n");
    break;
  case 0xc:
    printf("1st-level data cache: 16KB, 4-way set assoc, 32 byte line size\n");
    break;
  case 0xd:
    printf("1st-level data cache: 16KB, 4-way set assoc, 64 byte line size, ECC\n");
    break;
  case 0x21:
    printf("256-KB L2 (MLC), 8-way set associative, 64 byte line size\n");
    break;
  case 0x22:
    printf("3rd-level cache: 512 KB, 4-way set associative, sectored cache, 64-byte line size\n");
    break;
  case 0x23:
    printf("3rd-level cache: 1-MB, 8-way set associative, sectored cache, 64-byte line size\n");
    break;
  case 0x25:
    printf("3rd-level cache: 2-MB, 8-way set associative, sectored cache, 64-byte line size\n");
    break;
  case 0x29:
    printf("3rd-level cache: 4-MB, 8-way set associative, sectored cache, 64-byte line size\n");
    break;
  case 0x2c:
    printf("1st-level data cache: 32-KB, 8-way set associative, 64-byte line size\n");
    break;
  case 0x30:
    printf("1st-level instruction cache: 32-KB, 8-way set associative, 64-byte line size\n");
    break;
  case 0x39:
    printf("2nd-level cache: 128-KB, 4-way set associative, sectored cache, 64-byte line size\n");
    break;
  case 0x3a:
    printf("2nd-level cache: 192-KB, 6-way set associative, sectored cache, 64-byte line size\n");
    break;
  case 0x3b:
    printf("2nd-level cache: 128-KB, 2-way set associative, sectored cache, 64-byte line size\n");
    break;
  case 0x3c:
    printf("2nd-level cache: 256-KB, 4-way set associative, sectored cache, 64-byte line size\n");
    break;
  case 0x3d:
    printf("2nd-level cache: 384-KB, 6-way set associative, sectored cache, 64-byte line size\n");
    break;
  case 0x3e:
    printf("2nd-level cache: 512-KB, 4-way set associative, sectored cache, 64-byte line size\n");
    break;
  case 0x40:
    printf("No 2nd-level cache, or if 2nd-level cache exists, no 3rd-level cache\n");
    break;
  case 0x41:
    printf("2nd-level cache: 128KB, 4-way set assoc, 32 byte line size\n");
    break;
  case 0x42:
    printf("2nd-level cache: 256KB, 4-way set assoc, 32 byte line size\n");
    break;
  case 0x43:
    printf("2nd-level cache: 512KB, 4-way set assoc, 32 byte line size\n");
    break;
  case 0x44:
    printf("2nd-level cache: 1MB, 4-way set assoc, 32 byte line size\n");
    break;
  case 0x45:
    printf("2nd-level cache: 2MB, 4-way set assoc, 32 byte line size\n");
    break;
  case 0x46:
    printf("3rd-level cache: 4MB, 4-way set associative, 64-byte line size\n");
    break;
  case 0x47:
    printf("3rd-level cache: 8MB, 8-way set associative, 64-byte line size\n");
    break;
  case 0x48:
    printf("2nd-level cache: 3MB, 12-way set associative, 64-byte line size, unified on die\n");
    break;
  case 0x49:
    /* TODO The code needs to be slightly restructured so we can check family and model here */
    printf("3rd-level cache: 4MB, 16-way set associative, 64-byte line size (Intel Xeon MP, Family 0Fh, Model 06h\n");
    printf("OR 2nd-level cache: 4MB, 16-way set associative, 64-byte line size\n");
    break;
  case 0x4a:
    printf("3rd-level cache: 6MB, 12-way set associative, 64-byte line size\n");
    break;
  case 0x4b:
    printf("3rd-level cache: 8MB, 16-way set associative, 64-byte line size\n");
    break;
  case 0x4c:
    printf("3rd-level cache: 12MB, 12-way set associative, 64-byte line size\n");
    break;
  case 0x4d:
    printf("3rd-level cache: 16MB, 16-way set associative, 64-byte line size\n");
    break;
  case 0x4e:
    printf("3rd-level cache: 6MB, 24-way set associative, 64-byte line size\n");
    break;
  case 0x50:
    printf("Instruction TLB: 4KB, 2MB or 4MB pages, fully assoc., 64 entries\n");
    break;
  case 0x51:
    printf("Instruction TLB: 4KB, 2MB or 4MB pages, fully assoc., 128 entries\n");
    break;
  case 0x52:
    printf("Instruction TLB: 4KB, 2MB or 4MB pages, fully assoc., 256 entries\n");
    break;
  case 0x55:
    printf("Instruction TLB: 2MB or 4MB pages, fully assoc., 7 entries\n");
    break;
  case 0x56:
    printf("Data TLB: 4MB pages, 4-way set associative, 16 entries\n");
    break;
  case 0x57:
    printf("Data TLB: 4KB pages, 4-way set associative, 16 entries\n");
    break;
  case 0x5a:
    printf("Data TLB: 2MB or 4MB pages, 4-way set associative, 32 entries\n");
    break;
  case 0x5b:
    printf("Data TLB: 4KB or 4MB pages, fully assoc., 64 entries\n");
    break;
  case 0x5c:
    printf("Data TLB: 4KB or 4MB pages, fully assoc., 128 entries\n");
    break;
  case 0x5d:
    printf("Data TLB: 4KB or 4MB pages, fully assoc., 256 entries\n");
    break;
  case 0x60:
    printf("1st-level data cache: 16-KB, 8-way set associative, sectored cache, 64-byte line size\n");
    break;
  case 0x66:
    printf("1st-level data cache: 8KB, 4-way set assoc, 64 byte line size\n");
    break;
  case 0x67:
    printf("1st-level data cache: 16KB, 4-way set assoc, 64 byte line size\n");
    break;
  case 0x68:
    printf("1st-level data cache: 32KB, 4-way set assoc, 64 byte line size\n");
    break;
  case 0x70:
    printf("Trace cache: 12K-micro-op, 8-way set assoc\n");
    break;
  case 0x71:
    printf("Trace cache: 16K-micro-op, 8-way set assoc\n");
    break;
  case 0x72:
    printf("Trace cache: 32K-micro-op, 8-way set assoc\n");
    break;
  case 0x73:
    printf("Trace cache: 64K-micro-op, 8-way set assoc\n");
    break;
  case 0x76:
    printf("Instruction TLB: 2M/4M pages, fully associative, 8 entries\n");
    break;
  case 0x78:
    printf("2nd-level cache: 1MB, 4-way set assoc, 64 byte line size\n");
    break;
  case 0x79:
    printf("2nd-level cache: 128KB, 8-way set assoc, sectored, 64 byte line size\n");
    break;
  case 0x7a:
    printf("2nd-level cache: 256KB, 8-way set assoc, sectored, 64 byte line size\n");
    break;
  case 0x7b:
    printf("2nd-level cache: 512KB, 8-way set assoc, sectored, 64 byte line size\n");
    break;
  case 0x7c:
    printf("2nd-level cache: 1MB, 8-way set assoc, sectored, 64 byte line size\n");
    break;
  case 0x7d:
    printf("2nd-level cache: 2-MB, 8-way set associative, 64-byte line size\n");
    break;
  case 0x7f:
    printf("2nd-level cache: 512KB, 2-way set assoc, 64 byte line size\n");
    break;
  case 0x82:
    printf("2nd-level cache: 256KB, 8-way set assoc, 32 byte line size\n");
    break;
  case 0x83:
    printf("2nd-level cache: 512KB, 8-way set assoc 32 byte line size\n");
    break;
  case 0x84:
    printf("2nd-level cache: 1MB, 8-way set assoc, 32 byte line size\n");
    break;
  case 0x85:
    printf("2nd-level cache: 2MB, 8-way set assoc, 32 byte line size\n");
    break;
  case 0x86:
    printf("2nd-level cache: 512KB, 4-way set assoc, 64 byte line size\n");
    break;
  case 0x87:
    printf("2nd-level cache: 1MB, 8-way set assoc, 64 byte line size\n");
    break;
  case 0xB0:
    printf("Instruction TLB: 4-KB Pages, 4-way set associative, 128 entries\n");
    break;
  case 0xB1:
    printf("Instruction TLB: 2MB Pages (8 entries) or 4MB pages (4 entries), 4-way set associative\n");
    break;
  case 0xB2:
    printf("Instruction TLB: 4-KB Pages, 4-way set associative, 64 entries\n");
    break;
  case 0xB3:
    printf("Data TLB: 4-KB Pages, 4-way set associative, 128 entries\n");
    break;
  case 0xB4:
    printf("Data TLB: 4-KB Pages, 4-way set associative, 256 entries\n");
    break;
  case 0xCA:
    printf("Shared 2nd-level TLB: 4-KB Pages, 4-way set associative, 512 entries\n");
    break;
  case 0xD0:
    printf("3rd-level cache: 512KB, 4-way set associative, 64-byte line size\n");
    break;
  case 0xD1:
    printf("3rd-level cache: 1MB, 4-way set associative, 64-byte line size\n");
    break;
  case 0xD2:
    printf("3rd-level cache: 2MB, 4-way set associative, 64-byte line size\n");
    break;
  case 0xD6:
    printf("3rd-level cache: 1MB, 8-way set associative, 64-byte line size\n");
    break;
  case 0xD7:
    printf("3rd-level cache: 2MB, 8-way set associative, 64-byte line size\n");
    break;
  case 0xD8:
    printf("3rd-level cache: 4MB, 8-way set associative, 64-byte line size\n");
    break;
  case 0xDC:
    printf("3rd-level cache: 1.5MB, 12-way set associative, 64-byte line size\n");
    break;
  case 0xDD:
    printf("3rd-level cache: 3MB, 12-way set associative, 64-byte line size\n");
    break;
  case 0xDE:
    printf("3rd-level cache: 6MB, 12-way set associative, 64-byte line size\n");
    break;
  case 0xE2:
    printf("3rd-level cache: 2MB, 16-way set associative, 64-byte line size\n");
    break;
  case 0xE3:
    printf("3rd-level cache: 4MB, 16-way set associative, 64-byte line size\n");
    break;
  case 0xE4:
    printf("3rd-level cache: 8MB, 16-way set associative, 64-byte line size\n");
    break;
  case 0xEA:
    printf("3rd-level cache: 12MB, 24-way set associative, 64-byte line size\n");
    break;
  case 0xEB:
    printf("3rd-level cache: 18MB, 24-way set associative, 64-byte line size\n");
    break;
  case 0xEC:
    printf("3rd-level cache: 24MB, 24-way set associative, 64-byte line size\n");
    break;
  case 0xF0:
    printf("64-byte prefetching\n");
    break;
  case 0xF1:
    printf("128-byte prefetching\n");
    break;
  case 0xFF:
    printf("CPUID leaf 2 does not report full information, query leaf 4\n");
    break;
   default:
    printf("unknown TLB/cache descriptor\n");
    break;
  }
}
char *AMD_feature_flags[] = {
  "FPU    Floating Point Unit",
  "VME    Virtual 8086 Mode Enhancements",
  "DE     Debugging Extensions",
  "PSE    Page Size Extensions",
  "TSC    Time Stamp Counter",
  "MSR    Model Specific Registers",
  "PAE    Physical Address Extension",
  "MCE    Machine Check Exception",
  "CX8    COMPXCHG8B Instruction",
  "APIC   On-chip Advanced Programmable Interrupt Controller present and enabled",
  "10     Reserved",
  "SEP    Fast System Call",
  "MTRR   Memory Type Range Registers",
  "PGE    PTE Global Flag",
  "MCA    Machine Check Architecture",
  "CMOV   Conditional Move and Compare Instructions",
  "PAT    Page Attribute Table",
  "PSE36  36-bit Page Size Extension",
  "18     Reserved",
  "CLFSH  CLFLUSH instruction",
  "20     Reserved",
  "21     Reserved",
  "22     Reserved",
  "MMX    MMX instruction set",
  "FXSR   Fast FP/MMX Streaming SIMD Extensions save/restore",
  "SSE    SSE extensions",
  "SSE2   SSE2 extensions",
  "27     Reserved",
  "HTT    Hyper-Threading Technology",
  "29     Reserved",
  "30     Reserved",
  "31     Reserved",
};

char *AMD_feature_flags2[] = {
  "FPU    Floating Point Unit",
  "VME    Virtual 8086 Mode Enhancements",
  "DE     Debugging Extensions",
  "PSE    Page Size Extensions",
  "TSC    Time Stamp Counter",
  "MSR    Model Specific Registers",
  "PAE    Physical Address Extension",
  "MCE    Machine Check Exception",
  "CX8    COMPXCHG8B Instruction",
  "APIC   On-chip Advanced Programmable Interrupt Controller present and enabled",
  "10     Reserved",
  "SEP    Fast System Call",
  "MTRR   Memory Type Range Registers",
  "PGE    PTE Global Flag",
  "MCA    Machine Check Architecture",
  "CMOV   Conditional Move and Compare Instructions",
  "PAT    Page Attribute Table",
  "PSE36  36-bit Page Size Extension",
  "18     Reserved",
  "19     Reserved",
  "NX     No-execute page protection",
  "21     Reserved",
  "MmxExt MMX instruction extensions",
  "MMX    MMX instructions",
  "FXSR   Fast FP/MMX Streaming SIMD Extensions save/restore",
  "FFXSR  FXSAVE and FXRSTOR instruction optimizations",
  "Pge1GB 1GB Page Support",
  "RDTSCP RDTSCP instruction",
  "28     Reserved",
  "LM     64 bit long mode",
  "3DNowE 3DNow! instruction extensions",
  "3DNow  3DNow! instructions",
};

char *AMD_feature_flags3[] = {
  "LhfSaf LAHF and SAHF instructions in 65-bit mode",
  "CmpLeg Core Multi-Processing mode",
  "SVM    Secure Virtual Machine",
  "XAPSPC Extended APIC Register Space",
  "AltMC8 LOCK MOV CR0 means MOV CR8",
  "ABM    Advanced Bit Manipulation",
  "SSE4A  EXTRQ, INSERTQ, MOVNTSS, and MOVNTSD support",
  "MASSE  Misaligned SSE mode",
  "3DNPFC PREFETCH and PREFETCHW support",
  "OSVW   OS Visible Workaround support",
  "10     Reserved",
  "11     Reserved",
  "SKINIT SKINIT, STGI, and DEV support",
  "WDT    Watchdog Timer support"
  "14     Reserved",
  "15     Reserved",
  "16     Reserved",
  "17     Reserved",
  "18     Reserved",
  "19     Reserved",
  "20     Reserved",
  "21     Reserved",
  "22     Reserved",
  "23     Reserved",
  "24     Reserved",
  "25     Reserved",
  "26     Reserved",
  "27     Reserved",
  "28     Reserved",
  "29     Reserved",
  "30     Reserved",
  "31     Reserved",
};

char *Assoc[] = {
  "L2 off",
  "Direct mapped",
  "2-way",
  "reserved",
  "4-way",
  "reserved",
  "8-way",
  "reserved",
  "16-way",
  "reserved",
  "reserved",
  "reserved",
  "reserved",
  "reserved",
  "reserved",
  "full",
};



/* AMD-specific information */
void doamd(int maxi){
  unsigned long maxei,unused;
  int family = 0;

  printf("AMD-specific functions\n");

  /* Do standard stuff */
  if(maxi >= 1){
    unsigned long eax,ebx,edx,unused;
    int stepping,model,reserved;

    cpuid(1,eax,ebx,unused,edx);
    stepping = eax & 0xf;
    model = (eax >> 4) & 0xf;
    family = (eax >> 8) & 0xf;
    reserved = eax >> 12;

    printf("Version %08lx:\n",eax);
    printf("Family: %d Model: %d [",family,model);
    switch(family){
    case 4:
      printf("486 model %d",model);
      break;
    case 5:
      switch(model){
      case 0:
      case 1:
      case 2:
      case 3:
      case 6:
      case 7:
	printf("K6 Model %d",model);
	break;
      case 8:
	printf("K6-2 Model 8");
	break;
      case 9:
	printf("K6-III Model 9");
	break;
      default:
	printf("K5/K6 model %d",model);
	break;
      }
      break;
    case 6:
      switch(model){
      case 1:
      case 2:
      case 4:
	printf("Athlon model %d",model);
	break;
      case 3:
	printf("Duron model 3");
	break;
      case 6:
	printf("Athlon MP/Mobile Athlon model 6");
	break;
      case 7:
	printf("Mobile Duron Model 7");
	break;
      default:
	printf("Duron/Athlon model %d",model);
	break;
      }

      break;
    }
    printf("]\n\n");
    {
      int i;

      printf("Standard feature flags %08lx:\n",edx);
      for(i=0;i<32;i++){
	if(family == 5 && model == 0){
	  if(i == 9)
	    printf("Global Paging Extensions\n");
	  else if(i == 13)
	    printf("13 - reserved\n");
	} else {
	  if(edx & (1<<i)){
	    printf("%s\n",AMD_feature_flags[i]);
	  }
	}
      }
    }
  }
  /* Check for presence of extended info */
  cpuid(0x80000000,maxei,unused,unused,unused);
  if(maxei == 0)
    return;

  if(maxei >= 0x80000001){
    unsigned long eax,ebx,ecx,edx;
    int stepping,model,generation,reserved;
    int i;

    cpuid(0x80000001,eax,ebx,ecx,edx);
    stepping = eax & 0xf;
    model = (eax >> 4) & 0xf;
    generation = (eax >> 8) & 0xf;
    reserved = eax >> 12;

    printf("Generation: %d Model: %d\n",generation,model);
    printf("Extended feature flags %08lx:\n",edx);
    for(i=0;i<32;i++){
      if(family == 5 && model == 0 && i == 9){
	printf("Global Paging Extensions\n");
      } else {
	if(edx & (1<<i)){
	  printf("%s\n",AMD_feature_flags2[i]);
	}
      }
    }
    printf("\nExtended Miscellaneous feature flags %08lx:\n", ecx);
    for(i=0;i<32;i++){
      if(ecx & (1<<i)){
        printf("%s\n",AMD_feature_flags3[i]);
      }
    }
  }
  printf("\n");
  if(maxei >= 0x80000002){
    /* Processor identification string */
    int j;
    printf("Processor name string: ");
    for(j=0x80000002;j<=0x80000004;j++){
      unsigned long eax,ebx,ecx,edx;

      cpuid(j,eax,ebx,ecx,edx);
      printregs(eax,ebx,ecx,edx);
    }
    printf("\n");
  }
  if(maxei >= 0x80000005){
    /* TLB and cache info */
    unsigned long eax,ebx,ecx,edx;

    cpuid(0x80000005,eax,ebx,ecx,edx);
    printf("L1 Cache Information:\n");
    if(family >= 6){
      printf("2/4-MB Pages:\n");
      printf("   Data TLB: associativity %ld-way #entries %ld\n",
	     (eax >> 24) & 0xff,(eax >> 16) & 0xff);
      printf("   Instruction TLB: associativity %ld-way #entries %ld\n",
	     (eax >> 8) & 0xff,eax & 0xff);
    }
    printf("4-KB Pages:\n");
    printf("   Data TLB: associativity %ld-way #entries %ld\n",
	   (ebx >> 24) & 0xff,(ebx >> 16) & 0xff);
    printf("   Instruction TLB: associativity %ld-way #entries %ld\n",
	   (ebx >> 8) & 0xff,ebx & 0xff);

    printf("L1 Data cache:\n");
    printf("   size %ld KB associativity %lx-way lines per tag %ld line size %ld\n",
	   ecx >> 24,(ecx>>16) & 0xff,(ecx >> 8)&0xff,ecx&0xff);

    printf("L1 Instruction cache:\n");
    printf("   size %ld KB associativity %lx-way lines per tag %ld line size %ld\n",
	   edx >> 24,(edx>>16) & 0xff,(edx >> 8)&0xff,edx&0xff);
    printf("\n");
  }

  /* check K6-III (and later?) on-chip L2 cache size */
  if (maxei >= 0x80000006) {
    unsigned long eax,ebx,ecx,unused;
    int assoc;

    cpuid(0x80000006,eax,ebx,ecx,unused);
    printf("L2 Cache Information:\n");
    if(family >= 6){
      printf("2/4-MB Pages:\n");
      assoc = (eax >> 24) & 0xff;
      if(assoc == 6)
	assoc = 8;
      printf("   Data TLB: associativity %s #entries %ld\n",
	     Assoc[(eax >> 24) & 0xf],(eax >> 16) & 0xff);
      assoc = (eax >> 16) & 0xff;
      printf("   Instruction TLB: associativity %s #entries %ld\n",
	     Assoc[(eax >> 8) & 0xf],eax & 0xff);

      printf("4-KB Pages:\n");
      printf("   Data TLB: associativity %s #entries %ld\n",
	     Assoc[(ebx >> 24) & 0xf],(ebx >> 16) & 0xff);
      printf("   Instruction TLB: associativity %s #entries %ld\n",
	     Assoc[(ebx >> 8) & 0xf],ebx & 0xff);
    }
    printf("   size %ld KB associativity %s lines per tag %ld line size %ld\n",
	   ecx >> 24,Assoc[(ecx>>16) & 0xf],(ecx >> 8)&0xff,ecx&0xff);

    printf("\n");
  }
  /* Check power management feature flags */
  if(maxei >= 0x80000007){
    unsigned long unused,edx;

    printf("Advanced Power Management Feature Flags\n");
    cpuid(0x80000007,unused,unused,unused,edx);
    if(edx & 1)
      printf("Has temperature sensing diode\n");
    if(edx & 2)
      printf("Supports Frequency ID control\n");
    if(edx & 4)
      printf("Supports Voltage ID control\n");
  }
  /* Check phys address & linear address size */
  if(maxei >= 0x80000008){
    unsigned long unused,eax;

    cpuid(0x80000008,eax,unused,unused,unused);
    printf("Maximum linear address: %ld; maximum phys address %ld\n",
	   (eax>>8) & 0xff,eax&0xff);
  }
}

char *Cyrix_standard_feature_flags_5[] = {
  "FPU   Floating Point Unit",
  "V86   Virtual Mode Extensions",
  "Debug Extension",
  "4MB Page Size",
  "Time Stamp Counter",
  "RDMSR/WRMSR (Model Specific Registers)",
  "PAE",
  "Machine Check Exception",
  "COMPXCHG8B Instruction",
  "APIC - On-chip Advanced Programmable Interrupt Controller present and enabled",
  "10 - Reserved",
  "11 - Reserved",
  "MTRR  Memory Type Range Registers",
  "13 - reserved",
  "Machine Check",
  "CMOV  Conditional Move Instruction",
  "16 - reserved",
  "17 - reserved",
  "18 - reserved",
  "19 - reserved",
  "20 - reserved",
  "21 - reserved",
  "22 - reserved",
  "MMX instructions",
  "24 - reserved",
  "25 - reserved",
  "26 - reserved",
  "27 - reserved",
  "28 - reserved",
  "29 - reserved",
  "30 - reserved",
};

char *Cyrix_standard_feature_flags_not5[] = {
  "FPU   Floating Point Unit",
  "V86   Virtual Mode Extensions",
  "Debug Extension",
  "4MB Page Size",
  "Time Stamp Counter",
  "RDMSR/WRMSR (Model Specific Registers)",
  "PAE",
  "Machine Check Exception",
  "COMPXCHG8B Instruction",
  "APIC - On-chip Advanced Programmable Interrupt Controller present and enabled",
  "10 - Reserved",
  "11 - Reserved",
  "MTRR  Memory Type Range Registers",
  "Global Paging Extension",
  "Machine Check",
  "CMOV  Conditional Move Instruction",
  "16 - reserved",
  "17 - reserved",
  "18 - reserved",
  "19 - reserved",
  "20 - reserved",
  "21 - reserved",
  "22 - reserved",
  "MMX instructions",
  "24 - reserved",
  "25 - reserved",
  "26 - reserved",
  "27 - reserved",
  "28 - reserved",
  "29 - reserved",
  "30 - reserved",
};


char *Cyrix_extended_feature_flags[] = {
  "FPU   Floating Point Unit",
  "V86   Virtual Mode Extensions",
  "Debug Extension",
  "Page Size Extensions",
  "Time Stamp Counter",
  "Cyrix MSR",
  "PAE",
  "MC Exception",
  "COMPXCHG8B",
  "APIC on chip",
  "SYSCALL/SYSRET",
  "11 - reserved",
  "MTRR",
  "Global bit",
  "Machine Check",
  "CMOV",
  "FPU CMOV",
  "17 - reserved",
  "18 - reserved",
  "19 - reserved",
  "20 - reserved",
  "21 - reserved",
  "22 - reserved",
  "MMX",
  "Extended MMX",
  "25 - reserved",
  "26 - reserved",
  "27 - reserved",
  "28 - reserved",
  "29 - reserved",
  "30 - reserved",
  "3DNow instructions",
};

/* Cyrix-specific information */
void docyrix(int maxi){
  unsigned long maxei,unused;
  int i;

  printf("Cyrix-specific functions\n");
  cpuid(0x80000000,maxei,unused,unused,unused);
  /* Dump extended info, if any, in raw hex */
  for(i=0x80000000;i<=maxei;i++){
    unsigned long eax,ebx,ecx,edx;

    cpuid(i,eax,ebx,ecx,edx);
    printf("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n",i,eax,ebx,ecx,edx);
  }

  /* Do standard stuff */
  if(maxi >= 1){
    unsigned long eax,unused,edx;
    int stepping,model,family,reserved;

    cpuid(1,eax,unused,unused,edx);
    stepping = eax & 0xf;
    model = (eax >> 4) & 0xf;
    family = (eax >> 8) & 0xf;
    reserved = eax >> 12;

    printf("Family: %d Model: %d [",family,model);
    switch(family){
    case 4:
      switch(model){
      case 4:
	printf("MediaGX");
	break;
      }
      break;
    case 5:
      switch(model){
      case 2:
	printf("6x86");
	break;
      case 4:
	printf("BXm");
	break;
      }
      break;
    case 6:
      switch(model){
      case 0:
	printf("6x86/MX");
	break;
      }
      break;
    }
    printf("]\n\n");
    if(family == 5 && model == 0){
      int i;

      for(i=0;i<32;i++){
	if(edx & (1<<i)){
	  printf("%s\n",Cyrix_standard_feature_flags_5[i]);
	}
      }
    } else {
      int i;

      for(i=0;i<32;i++){
	if(edx & (1<<i)){
	  printf("%s\n",Cyrix_standard_feature_flags_not5[i]);
	}
      }
    }
  }
  if(maxi >= 2){
    /* TLB and L1 Cache info */
    int ntlb = 255;
    int i;

    for(i=0;i<ntlb;i++){
      unsigned long eax,edx,unused;

      cpuid(2,eax,unused,unused,edx);
      ntlb =  eax & 0xff;
      decode_cyrix_tlb(eax >> 8);
      decode_cyrix_tlb(eax >> 16);
      decode_cyrix_tlb(eax >> 24);

      /* ebx and ecx are reserved */

      if((edx & 0x80000000) == 0){
	decode_cyrix_tlb(edx);
	decode_cyrix_tlb(edx >> 8);
	decode_cyrix_tlb(edx >> 16);
	decode_cyrix_tlb(edx >> 24);
      }
    }
  }

  /* Check for presence of extended info */
  if(maxei < 0x80000000)
    return;

  printf("\nExtended info:\n");
  if(maxei >= 0x80000001){
    unsigned long eax,ebx,ecx,edx;
    int stepping,model,family,reserved,i;

    cpuid(0x80000001,eax,ebx,ecx,edx);
    stepping = eax & 0xf;
    model = (eax >> 4) & 0xf;
    family = (eax >> 8) & 0xf;
    reserved = eax >> 12;
    printf("Family: %d Model: %d [",family,model);
    switch(family){
    case 4:
      printf("MediaGX");
      break;
    case 5:
      printf("6x86/GXm");
      break;
    case 6:
      printf("6x86/MX");
    }
    printf("]\n\n");

    printf("Extended feature flags:\n");
    for(i=0;i<32;i++){
      if(edx & (1<<i)){
	printf("%s\n",Cyrix_extended_feature_flags[i]);
      }
    }
  }
  printf("\n");
  if(maxei >= 0x80000002){
    /* Processor identification string */
    char namestring[49],*cp;
    int j;
    cp = namestring;
    printf("Processor name string: ");
    for(j=0x80000002;j<=0x80000004;j++){
      unsigned long eax,ebx,ecx,edx;

      cpuid(j,eax,ebx,ecx,edx);
      printregs(eax,ebx,ecx,edx);
    }
  }
  if(maxei >= 0x80000005){
    /* TLB and L1 Cache info */
    int ntlb = 255;
    int i;

    for(i=0;i<ntlb;i++){
      unsigned long eax,ebx,ecx,unused;

      cpuid(0x80000005,eax,ebx,ecx,unused);
      ntlb =  eax & 0xff;
      decode_cyrix_tlb(ebx >> 8);
      decode_cyrix_tlb(ebx >> 16);
      decode_cyrix_tlb(ebx >> 24);

      /* eax and edx are reserved */

      if((ecx & 0x80000000) == 0){
	decode_cyrix_tlb(ecx);
	decode_cyrix_tlb(ecx >> 8);
	decode_cyrix_tlb(ecx >> 16);
	decode_cyrix_tlb(ecx >> 24);
      }
    }
  }
}


/* Decode Cyrix TLB and cache info descriptors */
void decode_cyrix_tlb(int x){
  switch(x & 0xff){
  case 0:
    break;
  case 0x70:
    printf("TLB: 32 entries 4-way associative 4KB pages\n");
    break;
  case 0x80:
    printf("L1 Cache: 16KB 4-way associative 16 bytes/line\n");
    break;
  }
}
