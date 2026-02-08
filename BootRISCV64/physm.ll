; ModuleID = 'physm.cpp'
source_filename = "physm.cpp"
target datalayout = "e-m:e-p:64:64-i64:64-i128:128-n32:64-S128"
target triple = "riscv64-pc-windows-msvc19.33.0"

%struct._efi_memory_map_ = type { i64, ptr, i64, i64, i32 }
%struct.EFI_MEMORY_DESCRIPTOR = type { i32, i64, i64, i64, i64 }

$"??$raw_offset@PEA_KPEAX@@YAPEA_KPEAXJ@Z" = comdat any

$"??$raw_diff@UEFI_MEMORY_DESCRIPTOR@@U1@@@YAJPEAUEFI_MEMORY_DESCRIPTOR@@0@Z" = comdat any

$"??$raw_diff@_K_K@@YAJPEA_K0@Z" = comdat any

$"??$raw_offset@PEAUEFI_MEMORY_DESCRIPTOR@@PEAU1@@@YAPEAUEFI_MEMORY_DESCRIPTOR@@PEAU0@J@Z" = comdat any

$"??_C@_0BA@KEPMIDLN@Address?5?9?$DO?5?$CFx?5?6?$AA@" = comdat any

@"?pagestack@@3PEA_KEA" = dso_local global ptr null, align 8
@"?stackptr@@3PEA_KEA" = dso_local global ptr null, align 8
@"?allocatedStack@@3PEA_KEA" = dso_local global ptr null, align 8
@"?allocatedPtr@@3PEA_KEA" = dso_local global ptr null, align 8
@"?allocatedCount@@3IA" = dso_local global i32 0, align 4
@"?usableRam@@3_KA" = dso_local global i64 0, align 8
@"?usableSize@@3_KA" = dso_local global i64 0, align 8
@"?ramSize@@3_KA" = dso_local global i64 0, align 8
@"??_C@_0BA@KEPMIDLN@Address?5?9?$DO?5?$CFx?5?6?$AA@" = linkonce_odr dso_local unnamed_addr constant [16 x i8] c"Address -> %x \0A\00", comdat, align 1

; Function Attrs: mustprogress noinline optnone
define dso_local void @"?XEInitialisePmmngr@@YAXU_efi_memory_map_@@PEAXK@Z"(ptr noundef %0, ptr noundef %1, i64 noundef %2) #0 {
  %4 = alloca i64, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i64, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i64, align 8
  %10 = alloca i64, align 8
  store i64 %2, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  store ptr %0, ptr %6, align 8
  store i64 0, ptr %7, align 8
  %11 = load ptr, ptr %5, align 8
  store ptr %11, ptr @"?pagestack@@3PEA_KEA", align 8
  store ptr %11, ptr @"?stackptr@@3PEA_KEA", align 8
  %12 = load i64, ptr %4, align 8
  %13 = udiv i64 %12, 2
  store i64 %13, ptr %4, align 8
  %14 = load i64, ptr %4, align 8
  %15 = load ptr, ptr %5, align 8
  %16 = call noundef ptr @"??$raw_offset@PEA_KPEAX@@YAPEA_KPEAXJ@Z"(ptr noundef %15, i64 noundef %14)
  store ptr %16, ptr @"?allocatedStack@@3PEA_KEA", align 8
  store ptr %16, ptr @"?allocatedPtr@@3PEA_KEA", align 8
  store i32 1, ptr @"?allocatedCount@@3IA", align 4
  %17 = getelementptr inbounds nuw %struct._efi_memory_map_, ptr %0, i32 0, i32 1
  %18 = load ptr, ptr %17, align 8
  store ptr %18, ptr %8, align 8
  br label %19

19:                                               ; preds = %81, %3
  %20 = getelementptr inbounds nuw %struct._efi_memory_map_, ptr %0, i32 0, i32 1
  %21 = load ptr, ptr %20, align 8
  %22 = load ptr, ptr %8, align 8
  %23 = call noundef i64 @"??$raw_diff@UEFI_MEMORY_DESCRIPTOR@@U1@@@YAJPEAUEFI_MEMORY_DESCRIPTOR@@0@Z"(ptr noundef %22, ptr noundef %21)
  %24 = getelementptr inbounds nuw %struct._efi_memory_map_, ptr %0, i32 0, i32 0
  %25 = load i64, ptr %24, align 8
  %26 = icmp ult i64 %23, %25
  br i1 %26, label %27, label %86

27:                                               ; preds = %19
  %28 = load ptr, ptr %8, align 8
  %29 = getelementptr inbounds nuw %struct.EFI_MEMORY_DESCRIPTOR, ptr %28, i32 0, i32 3
  %30 = load i64, ptr %29, align 8
  %31 = mul i64 %30, 4096
  %32 = load i64, ptr @"?ramSize@@3_KA", align 8
  %33 = add i64 %32, %31
  store i64 %33, ptr @"?ramSize@@3_KA", align 8
  %34 = load ptr, ptr %8, align 8
  %35 = getelementptr inbounds nuw %struct.EFI_MEMORY_DESCRIPTOR, ptr %34, i32 0, i32 0
  %36 = load i32, ptr %35, align 8
  %37 = icmp eq i32 %36, 7
  br i1 %37, label %38, label %81

38:                                               ; preds = %27
  %39 = load ptr, ptr %8, align 8
  %40 = getelementptr inbounds nuw %struct.EFI_MEMORY_DESCRIPTOR, ptr %39, i32 0, i32 1
  %41 = load i64, ptr %40, align 8
  store i64 %41, ptr %9, align 8
  %42 = load ptr, ptr %8, align 8
  %43 = getelementptr inbounds nuw %struct.EFI_MEMORY_DESCRIPTOR, ptr %42, i32 0, i32 3
  %44 = load i64, ptr %43, align 8
  store i64 %44, ptr %10, align 8
  %45 = load ptr, ptr %8, align 8
  %46 = getelementptr inbounds nuw %struct.EFI_MEMORY_DESCRIPTOR, ptr %45, i32 0, i32 1
  %47 = load i64, ptr %46, align 8
  store i64 %47, ptr @"?usableRam@@3_KA", align 8
  %48 = load ptr, ptr %8, align 8
  %49 = getelementptr inbounds nuw %struct.EFI_MEMORY_DESCRIPTOR, ptr %48, i32 0, i32 3
  %50 = load i64, ptr %49, align 8
  %51 = mul i64 %50, 4096
  store i64 %51, ptr @"?usableSize@@3_KA", align 8
  %52 = load i64, ptr %7, align 8
  %53 = add i64 %52, 1
  store i64 %53, ptr %7, align 8
  br label %54

54:                                               ; preds = %65, %38
  %55 = load i64, ptr %10, align 8
  %56 = icmp ugt i64 %55, 0
  br i1 %56, label %57, label %63

57:                                               ; preds = %54
  %58 = load ptr, ptr @"?pagestack@@3PEA_KEA", align 8
  %59 = load ptr, ptr @"?stackptr@@3PEA_KEA", align 8
  %60 = call noundef i64 @"??$raw_diff@_K_K@@YAJPEA_K0@Z"(ptr noundef %59, ptr noundef %58)
  %61 = load i64, ptr %4, align 8
  %62 = icmp ult i64 %60, %61
  br label %63

63:                                               ; preds = %57, %54
  %64 = phi i1 [ false, %54 ], [ %62, %57 ]
  br i1 %64, label %65, label %73

65:                                               ; preds = %63
  %66 = load i64, ptr %9, align 8
  %67 = load ptr, ptr @"?stackptr@@3PEA_KEA", align 8
  %68 = getelementptr inbounds nuw i64, ptr %67, i32 1
  store ptr %68, ptr @"?stackptr@@3PEA_KEA", align 8
  store i64 %66, ptr %67, align 8
  %69 = load i64, ptr %10, align 8
  %70 = add i64 %69, -1
  store i64 %70, ptr %10, align 8
  %71 = load i64, ptr %9, align 8
  %72 = add i64 %71, 4096
  store i64 %72, ptr %9, align 8
  br label %54, !llvm.loop !10

73:                                               ; preds = %63
  %74 = load ptr, ptr @"?pagestack@@3PEA_KEA", align 8
  %75 = load ptr, ptr @"?stackptr@@3PEA_KEA", align 8
  %76 = call noundef i64 @"??$raw_diff@_K_K@@YAJPEA_K0@Z"(ptr noundef %75, ptr noundef %74)
  %77 = load i64, ptr %4, align 8
  %78 = icmp uge i64 %76, %77
  br i1 %78, label %79, label %80

79:                                               ; preds = %73
  br label %86

80:                                               ; preds = %73
  br label %81

81:                                               ; preds = %80, %27
  %82 = getelementptr inbounds nuw %struct._efi_memory_map_, ptr %0, i32 0, i32 3
  %83 = load i64, ptr %82, align 8
  %84 = load ptr, ptr %8, align 8
  %85 = call noundef ptr @"??$raw_offset@PEAUEFI_MEMORY_DESCRIPTOR@@PEAU1@@@YAPEAUEFI_MEMORY_DESCRIPTOR@@PEAU0@J@Z"(ptr noundef %84, i64 noundef %83)
  store ptr %85, ptr %8, align 8
  br label %19, !llvm.loop !12

86:                                               ; preds = %79, %19
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone
define linkonce_odr dso_local noundef ptr @"??$raw_offset@PEA_KPEAX@@YAPEA_KPEAXJ@Z"(ptr noundef %0, i64 noundef %1) #1 comdat {
  %3 = alloca i64, align 8
  %4 = alloca ptr, align 8
  store i64 %1, ptr %3, align 8
  store ptr %0, ptr %4, align 8
  %5 = load ptr, ptr %4, align 8
  %6 = ptrtoint ptr %5 to i64
  %7 = load i64, ptr %3, align 8
  %8 = add i64 %6, %7
  %9 = inttoptr i64 %8 to ptr
  ret ptr %9
}

; Function Attrs: mustprogress noinline nounwind optnone
define linkonce_odr dso_local noundef i64 @"??$raw_diff@UEFI_MEMORY_DESCRIPTOR@@U1@@@YAJPEAUEFI_MEMORY_DESCRIPTOR@@0@Z"(ptr noundef %0, ptr noundef %1) #1 comdat {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %1, ptr %3, align 8
  store ptr %0, ptr %4, align 8
  %5 = load ptr, ptr %4, align 8
  %6 = ptrtoint ptr %5 to i64
  %7 = load ptr, ptr %3, align 8
  %8 = ptrtoint ptr %7 to i64
  %9 = sub nsw i64 %6, %8
  ret i64 %9
}

; Function Attrs: mustprogress noinline nounwind optnone
define linkonce_odr dso_local noundef i64 @"??$raw_diff@_K_K@@YAJPEA_K0@Z"(ptr noundef %0, ptr noundef %1) #1 comdat {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %1, ptr %3, align 8
  store ptr %0, ptr %4, align 8
  %5 = load ptr, ptr %4, align 8
  %6 = ptrtoint ptr %5 to i64
  %7 = load ptr, ptr %3, align 8
  %8 = ptrtoint ptr %7 to i64
  %9 = sub nsw i64 %6, %8
  ret i64 %9
}

; Function Attrs: mustprogress noinline nounwind optnone
define linkonce_odr dso_local noundef ptr @"??$raw_offset@PEAUEFI_MEMORY_DESCRIPTOR@@PEAU1@@@YAPEAUEFI_MEMORY_DESCRIPTOR@@PEAU0@J@Z"(ptr noundef %0, i64 noundef %1) #1 comdat {
  %3 = alloca i64, align 8
  %4 = alloca ptr, align 8
  store i64 %1, ptr %3, align 8
  store ptr %0, ptr %4, align 8
  %5 = load ptr, ptr %4, align 8
  %6 = ptrtoint ptr %5 to i64
  %7 = load i64, ptr %3, align 8
  %8 = add i64 %6, %7
  %9 = inttoptr i64 %8 to ptr
  ret ptr %9
}

; Function Attrs: mustprogress noinline nounwind optnone
define dso_local noundef i64 @"?XEPmmngrAllocate@@YA_KXZ"() #1 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  %3 = load ptr, ptr @"?stackptr@@3PEA_KEA", align 8
  %4 = load ptr, ptr @"?pagestack@@3PEA_KEA", align 8
  %5 = icmp eq ptr %3, %4
  br i1 %5, label %6, label %7

6:                                                ; preds = %0
  store i64 0, ptr %1, align 8
  br label %17

7:                                                ; preds = %0
  %8 = load ptr, ptr @"?stackptr@@3PEA_KEA", align 8
  %9 = getelementptr inbounds i64, ptr %8, i32 -1
  store ptr %9, ptr @"?stackptr@@3PEA_KEA", align 8
  %10 = load i64, ptr %9, align 8
  store i64 %10, ptr %2, align 8
  %11 = load i64, ptr %2, align 8
  %12 = load ptr, ptr @"?allocatedPtr@@3PEA_KEA", align 8
  %13 = getelementptr inbounds nuw i64, ptr %12, i32 1
  store ptr %13, ptr @"?allocatedPtr@@3PEA_KEA", align 8
  store i64 %11, ptr %12, align 8
  %14 = load i32, ptr @"?allocatedCount@@3IA", align 4
  %15 = add i32 %14, 1
  store i32 %15, ptr @"?allocatedCount@@3IA", align 4
  %16 = load i64, ptr %2, align 8
  store i64 %16, ptr %1, align 8
  br label %17

17:                                               ; preds = %7, %6
  %18 = load i64, ptr %1, align 8
  ret i64 %18
}

; Function Attrs: mustprogress noinline nounwind optnone
define dso_local void @"?XEPmmngrFree@@YAX_K@Z"(i64 noundef %0) #1 {
  %2 = alloca i64, align 8
  store i64 %0, ptr %2, align 8
  %3 = load i64, ptr %2, align 8
  %4 = load ptr, ptr @"?stackptr@@3PEA_KEA", align 8
  %5 = getelementptr inbounds nuw i64, ptr %4, i32 1
  store ptr %5, ptr @"?stackptr@@3PEA_KEA", align 8
  store i64 %3, ptr %4, align 8
  ret void
}

; Function Attrs: mustprogress noinline optnone
define dso_local void @"?XEPmmngrList@@YAXXZ"() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i64, align 8
  store i32 10, ptr %1, align 4
  br label %3

3:                                                ; preds = %6, %0
  %4 = load i32, ptr @"?allocatedCount@@3IA", align 4
  %5 = icmp ne i32 %4, 0
  br i1 %5, label %6, label %13

6:                                                ; preds = %3
  %7 = load ptr, ptr @"?allocatedPtr@@3PEA_KEA", align 8
  %8 = getelementptr inbounds i64, ptr %7, i32 -1
  store ptr %8, ptr @"?allocatedPtr@@3PEA_KEA", align 8
  %9 = load i64, ptr %7, align 8
  store i64 %9, ptr %2, align 8
  %10 = load i64, ptr %2, align 8
  call void (ptr, ...) @XEGuiPrint(ptr noundef @"??_C@_0BA@KEPMIDLN@Address?5?9?$DO?5?$CFx?5?6?$AA@", i64 noundef %10)
  %11 = load i32, ptr @"?allocatedCount@@3IA", align 4
  %12 = add i32 %11, -1
  store i32 %12, ptr @"?allocatedCount@@3IA", align 4
  br label %3, !llvm.loop !13

13:                                               ; preds = %3
  ret void
}

declare dso_local void @XEGuiPrint(ptr noundef, ...) #2

; Function Attrs: mustprogress noinline nounwind optnone
define dso_local noundef ptr @"?XEGetAlstack@@YAPEA_KXZ"() #1 {
  %1 = load ptr, ptr @"?allocatedStack@@3PEA_KEA", align 8
  ret ptr %1
}

; Function Attrs: mustprogress noinline nounwind optnone
define dso_local noundef ptr @"?XEGetAlstackptr@@YAPEA_KXZ"() #1 {
  %1 = load ptr, ptr @"?allocatedPtr@@3PEA_KEA", align 8
  ret ptr %1
}

; Function Attrs: mustprogress noinline nounwind optnone
define dso_local noundef i64 @"?XEReserveMemCount@@YA_KXZ"() #1 {
  %1 = load i32, ptr @"?allocatedCount@@3IA", align 4
  %2 = zext i32 %1 to i64
  ret i64 %2
}

; Function Attrs: mustprogress noinline nounwind optnone
define dso_local noundef ptr @"?XEGetPgStack@@YAPEA_KXZ"() #1 {
  %1 = load ptr, ptr @"?pagestack@@3PEA_KEA", align 8
  ret ptr %1
}

; Function Attrs: mustprogress noinline nounwind optnone
define dso_local noundef ptr @"?XEGetStackPtr@@YAPEA_KXZ"() #1 {
  %1 = load ptr, ptr @"?stackptr@@3PEA_KEA", align 8
  ret ptr %1
}

; Function Attrs: mustprogress noinline nounwind optnone
define dso_local noundef i64 @"?XEGetPhysicalStart@@YA_KXZ"() #1 {
  %1 = load i64, ptr @"?usableRam@@3_KA", align 8
  ret i64 %1
}

; Function Attrs: mustprogress noinline nounwind optnone
define dso_local noundef i64 @"?XEGetPhysicalSize@@YA_KXZ"() #1 {
  %1 = load i64, ptr @"?usableSize@@3_KA", align 8
  ret i64 %1
}

; Function Attrs: mustprogress noinline nounwind optnone
define dso_local noundef i64 @"?XEGetRamSize@@YA_KXZ"() #1 {
  %1 = load i64, ptr @"?ramSize@@3_KA", align 8
  ret i64 %1
}

attributes #0 = { mustprogress noinline optnone "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+64bit,+a,+c,+d,+f,+m,+relax,+zaamo,+zalrsc,+zca,+zcd,+zicsr,+zmmul,-b,-e,-experimental-p,-experimental-smctr,-experimental-ssctr,-experimental-svukte,-experimental-xqccmp,-experimental-xqcia,-experimental-xqciac,-experimental-xqcibi,-experimental-xqcibm,-experimental-xqcicli,-experimental-xqcicm,-experimental-xqcics,-experimental-xqcicsr,-experimental-xqciint,-experimental-xqciio,-experimental-xqcilb,-experimental-xqcili,-experimental-xqcilia,-experimental-xqcilo,-experimental-xqcilsm,-experimental-xqcisim,-experimental-xqcisls,-experimental-xqcisync,-experimental-xrivosvisni,-experimental-xrivosvizip,-experimental-xsfmclic,-experimental-xsfsclic,-experimental-zalasr,-experimental-zicfilp,-experimental-zicfiss,-experimental-zvbc32e,-experimental-zvkgs,-experimental-zvqdotq,-h,-q,-sdext,-sdtrig,-sha,-shcounterenw,-shgatpa,-shlcofideleg,-shtvala,-shvsatpa,-shvstvala,-shvstvecd,-smaia,-smcdeleg,-smcntrpmf,-smcsrind,-smdbltrp,-smepmp,-smmpm,-smnpm,-smrnmi,-smstateen,-ssaia,-ssccfg,-ssccptr,-sscofpmf,-sscounterenw,-sscsrind,-ssdbltrp,-ssnpm,-sspm,-ssqosid,-ssstateen,-ssstrict,-sstc,-sstvala,-sstvecd,-ssu64xl,-supm,-svade,-svadu,-svbare,-svinval,-svnapot,-svpbmt,-svvptc,-v,-xandesbfhcvt,-xandesperf,-xandesvbfhcvt,-xandesvdot,-xandesvpackfph,-xandesvsintload,-xcvalu,-xcvbi,-xcvbitmanip,-xcvelw,-xcvmac,-xcvmem,-xcvsimd,-xmipscbop,-xmipscmov,-xmipslsp,-xsfcease,-xsfmm128t,-xsfmm16t,-xsfmm32a16f,-xsfmm32a32f,-xsfmm32a8f,-xsfmm32a8i,-xsfmm32t,-xsfmm64a64f,-xsfmm64t,-xsfmmbase,-xsfvcp,-xsfvfnrclipxfqf,-xsfvfwmaccqqq,-xsfvqmaccdod,-xsfvqmaccqoq,-xsifivecdiscarddlone,-xsifivecflushdlone,-xtheadba,-xtheadbb,-xtheadbs,-xtheadcmo,-xtheadcondmov,-xtheadfmemidx,-xtheadmac,-xtheadmemidx,-xtheadmempair,-xtheadsync,-xtheadvdot,-xventanacondops,-xwchc,-za128rs,-za64rs,-zabha,-zacas,-zama16b,-zawrs,-zba,-zbb,-zbc,-zbkb,-zbkc,-zbkx,-zbs,-zcb,-zce,-zcf,-zclsd,-zcmop,-zcmp,-zcmt,-zdinx,-zfa,-zfbfmin,-zfh,-zfhmin,-zfinx,-zhinx,-zhinxmin,-zic64b,-zicbom,-zicbop,-zicboz,-ziccamoa,-ziccamoc,-ziccif,-zicclsm,-ziccrse,-zicntr,-zicond,-zifencei,-zihintntl,-zihintpause,-zihpm,-zilsd,-zimop,-zk,-zkn,-zknd,-zkne,-zknh,-zkr,-zks,-zksed,-zksh,-zkt,-ztso,-zvbb,-zvbc,-zve32f,-zve32x,-zve64d,-zve64f,-zve64x,-zvfbfmin,-zvfbfwma,-zvfh,-zvfhmin,-zvkb,-zvkg,-zvkn,-zvknc,-zvkned,-zvkng,-zvknha,-zvknhb,-zvks,-zvksc,-zvksed,-zvksg,-zvksh,-zvkt,-zvl1024b,-zvl128b,-zvl16384b,-zvl2048b,-zvl256b,-zvl32768b,-zvl32b,-zvl4096b,-zvl512b,-zvl64b,-zvl65536b,-zvl8192b" }
attributes #1 = { mustprogress noinline nounwind optnone "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+64bit,+a,+c,+d,+f,+m,+relax,+zaamo,+zalrsc,+zca,+zcd,+zicsr,+zmmul,-b,-e,-experimental-p,-experimental-smctr,-experimental-ssctr,-experimental-svukte,-experimental-xqccmp,-experimental-xqcia,-experimental-xqciac,-experimental-xqcibi,-experimental-xqcibm,-experimental-xqcicli,-experimental-xqcicm,-experimental-xqcics,-experimental-xqcicsr,-experimental-xqciint,-experimental-xqciio,-experimental-xqcilb,-experimental-xqcili,-experimental-xqcilia,-experimental-xqcilo,-experimental-xqcilsm,-experimental-xqcisim,-experimental-xqcisls,-experimental-xqcisync,-experimental-xrivosvisni,-experimental-xrivosvizip,-experimental-xsfmclic,-experimental-xsfsclic,-experimental-zalasr,-experimental-zicfilp,-experimental-zicfiss,-experimental-zvbc32e,-experimental-zvkgs,-experimental-zvqdotq,-h,-q,-sdext,-sdtrig,-sha,-shcounterenw,-shgatpa,-shlcofideleg,-shtvala,-shvsatpa,-shvstvala,-shvstvecd,-smaia,-smcdeleg,-smcntrpmf,-smcsrind,-smdbltrp,-smepmp,-smmpm,-smnpm,-smrnmi,-smstateen,-ssaia,-ssccfg,-ssccptr,-sscofpmf,-sscounterenw,-sscsrind,-ssdbltrp,-ssnpm,-sspm,-ssqosid,-ssstateen,-ssstrict,-sstc,-sstvala,-sstvecd,-ssu64xl,-supm,-svade,-svadu,-svbare,-svinval,-svnapot,-svpbmt,-svvptc,-v,-xandesbfhcvt,-xandesperf,-xandesvbfhcvt,-xandesvdot,-xandesvpackfph,-xandesvsintload,-xcvalu,-xcvbi,-xcvbitmanip,-xcvelw,-xcvmac,-xcvmem,-xcvsimd,-xmipscbop,-xmipscmov,-xmipslsp,-xsfcease,-xsfmm128t,-xsfmm16t,-xsfmm32a16f,-xsfmm32a32f,-xsfmm32a8f,-xsfmm32a8i,-xsfmm32t,-xsfmm64a64f,-xsfmm64t,-xsfmmbase,-xsfvcp,-xsfvfnrclipxfqf,-xsfvfwmaccqqq,-xsfvqmaccdod,-xsfvqmaccqoq,-xsifivecdiscarddlone,-xsifivecflushdlone,-xtheadba,-xtheadbb,-xtheadbs,-xtheadcmo,-xtheadcondmov,-xtheadfmemidx,-xtheadmac,-xtheadmemidx,-xtheadmempair,-xtheadsync,-xtheadvdot,-xventanacondops,-xwchc,-za128rs,-za64rs,-zabha,-zacas,-zama16b,-zawrs,-zba,-zbb,-zbc,-zbkb,-zbkc,-zbkx,-zbs,-zcb,-zce,-zcf,-zclsd,-zcmop,-zcmp,-zcmt,-zdinx,-zfa,-zfbfmin,-zfh,-zfhmin,-zfinx,-zhinx,-zhinxmin,-zic64b,-zicbom,-zicbop,-zicboz,-ziccamoa,-ziccamoc,-ziccif,-zicclsm,-ziccrse,-zicntr,-zicond,-zifencei,-zihintntl,-zihintpause,-zihpm,-zilsd,-zimop,-zk,-zkn,-zknd,-zkne,-zknh,-zkr,-zks,-zksed,-zksh,-zkt,-ztso,-zvbb,-zvbc,-zve32f,-zve32x,-zve64d,-zve64f,-zve64x,-zvfbfmin,-zvfbfwma,-zvfh,-zvfhmin,-zvkb,-zvkg,-zvkn,-zvknc,-zvkned,-zvkng,-zvknha,-zvknhb,-zvks,-zvksc,-zvksed,-zvksg,-zvksh,-zvkt,-zvl1024b,-zvl128b,-zvl16384b,-zvl2048b,-zvl256b,-zvl32768b,-zvl32b,-zvl4096b,-zvl512b,-zvl64b,-zvl65536b,-zvl8192b" }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+64bit,+a,+c,+d,+f,+m,+relax,+zaamo,+zalrsc,+zca,+zcd,+zicsr,+zmmul,-b,-e,-experimental-p,-experimental-smctr,-experimental-ssctr,-experimental-svukte,-experimental-xqccmp,-experimental-xqcia,-experimental-xqciac,-experimental-xqcibi,-experimental-xqcibm,-experimental-xqcicli,-experimental-xqcicm,-experimental-xqcics,-experimental-xqcicsr,-experimental-xqciint,-experimental-xqciio,-experimental-xqcilb,-experimental-xqcili,-experimental-xqcilia,-experimental-xqcilo,-experimental-xqcilsm,-experimental-xqcisim,-experimental-xqcisls,-experimental-xqcisync,-experimental-xrivosvisni,-experimental-xrivosvizip,-experimental-xsfmclic,-experimental-xsfsclic,-experimental-zalasr,-experimental-zicfilp,-experimental-zicfiss,-experimental-zvbc32e,-experimental-zvkgs,-experimental-zvqdotq,-h,-q,-sdext,-sdtrig,-sha,-shcounterenw,-shgatpa,-shlcofideleg,-shtvala,-shvsatpa,-shvstvala,-shvstvecd,-smaia,-smcdeleg,-smcntrpmf,-smcsrind,-smdbltrp,-smepmp,-smmpm,-smnpm,-smrnmi,-smstateen,-ssaia,-ssccfg,-ssccptr,-sscofpmf,-sscounterenw,-sscsrind,-ssdbltrp,-ssnpm,-sspm,-ssqosid,-ssstateen,-ssstrict,-sstc,-sstvala,-sstvecd,-ssu64xl,-supm,-svade,-svadu,-svbare,-svinval,-svnapot,-svpbmt,-svvptc,-v,-xandesbfhcvt,-xandesperf,-xandesvbfhcvt,-xandesvdot,-xandesvpackfph,-xandesvsintload,-xcvalu,-xcvbi,-xcvbitmanip,-xcvelw,-xcvmac,-xcvmem,-xcvsimd,-xmipscbop,-xmipscmov,-xmipslsp,-xsfcease,-xsfmm128t,-xsfmm16t,-xsfmm32a16f,-xsfmm32a32f,-xsfmm32a8f,-xsfmm32a8i,-xsfmm32t,-xsfmm64a64f,-xsfmm64t,-xsfmmbase,-xsfvcp,-xsfvfnrclipxfqf,-xsfvfwmaccqqq,-xsfvqmaccdod,-xsfvqmaccqoq,-xsifivecdiscarddlone,-xsifivecflushdlone,-xtheadba,-xtheadbb,-xtheadbs,-xtheadcmo,-xtheadcondmov,-xtheadfmemidx,-xtheadmac,-xtheadmemidx,-xtheadmempair,-xtheadsync,-xtheadvdot,-xventanacondops,-xwchc,-za128rs,-za64rs,-zabha,-zacas,-zama16b,-zawrs,-zba,-zbb,-zbc,-zbkb,-zbkc,-zbkx,-zbs,-zcb,-zce,-zcf,-zclsd,-zcmop,-zcmp,-zcmt,-zdinx,-zfa,-zfbfmin,-zfh,-zfhmin,-zfinx,-zhinx,-zhinxmin,-zic64b,-zicbom,-zicbop,-zicboz,-ziccamoa,-ziccamoc,-ziccif,-zicclsm,-ziccrse,-zicntr,-zicond,-zifencei,-zihintntl,-zihintpause,-zihpm,-zilsd,-zimop,-zk,-zkn,-zknd,-zkne,-zknh,-zkr,-zks,-zksed,-zksh,-zkt,-ztso,-zvbb,-zvbc,-zve32f,-zve32x,-zve64d,-zve64f,-zve64x,-zvfbfmin,-zvfbfwma,-zvfh,-zvfhmin,-zvkb,-zvkg,-zvkn,-zvknc,-zvkned,-zvkng,-zvknha,-zvknhb,-zvks,-zvksc,-zvksed,-zvksg,-zvksh,-zvkt,-zvl1024b,-zvl128b,-zvl16384b,-zvl2048b,-zvl256b,-zvl32768b,-zvl32b,-zvl4096b,-zvl512b,-zvl64b,-zvl65536b,-zvl8192b" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 21.1.8", isOptimized: false, runtimeVersion: 0, emissionKind: NoDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "physm.cpp", directory: "D:\\XenevaOS\\BootRISCV64")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 1, !"target-abi", !"lp64d"}
!5 = !{i32 6, !"riscv-isa", !6}
!6 = !{!"rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0_zmmul1p0_zaamo1p0_zalrsc1p0_zca1p0_zcd1p0"}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{i32 8, !"SmallDataLimit", i32 0}
!9 = !{!"clang version 21.1.8"}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
!12 = distinct !{!12, !11}
!13 = distinct !{!13, !11}
