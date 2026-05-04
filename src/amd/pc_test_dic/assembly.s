
/home/ge64cax2/GPUscout/inst-dir/tmp-gpuscout/reg2-spill-vec-offset8192-size42328.co:   file format elf64-amdgpu

Disassembly of section .text:

0000000000002400 <_Z14spillingKernelPfS_>:
; _Z14spillingKernelPfS_():
; /opt/rocm-7.1.0/lib/llvm/bin/../../../include/hip/amd_detail/amd_hip_runtime.h:264
; __DEVICE__ unsigned int __hip_get_block_dim_x() { return __ockl_get_local_size(0); }
        s_load_dword s6, s[4:5], 0x1c                              // 000000002400: C0020182 0000001C
        s_add_u32 s0, s0, s9                                       // 000000002408: 80000900
        s_addc_u32 s1, s1, 0                                       // 00000000240C: 82018001
        s_waitcnt lgkmcnt(0)                                       // 000000002410: BF8CC07F
        s_and_b32 s6, s6, 0xffff                                   // 000000002414: 8606FF06 0000FFFF
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:10
;     int idx = threadIdx.x + blockIdx.x * blockDim.x;
        s_mul_i32 s8, s8, s6                                       // 00000000241C: 92080608
        v_add_u32_e32 v0, s8, v0                                   // 000000002420: 68000008
        s_movk_i32 s6, 0x400                                       // 000000002424: B0060400
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:11
;     if (idx >= N) return;
        v_cmp_gt_i32_e32 vcc, s6, v0                               // 000000002428: 7D880006
        s_and_saveexec_b64 s[6:7], vcc                             // 00000000242C: BE86206A
        s_cbranch_execz L1                                         // 000000002430: BF880121
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:8
;     float temp[64]; // Adjust size based on register pressure
        s_load_dwordx4 s[4:7], s[4:5], 0x0                         // 000000002434: C00A0102 00000000
        v_ashrrev_i32_e32 v1, 31, v0                               // 00000000243C: 2202009F
        v_lshlrev_b64 v[0:1], 2, v[0:1]                            // 000000002440: D28F0000 00020082
        s_mov_b32 s8, 0                                            // 000000002448: BE880080
        s_mov_b32 s9, 0                                            // 00000000244C: BE890080
        s_waitcnt lgkmcnt(0)                                       // 000000002450: BF8CC07F
        v_mov_b32_e32 v3, s7                                       // 000000002454: 7E060207
        v_add_co_u32_e32 v2, vcc, s6, v0                           // 000000002458: 32040006
        v_addc_co_u32_e32 v3, vcc, v3, v1, vcc                     // 00000000245C: 38060303
        global_load_dword v2, v[2:3], off                          // 000000002460: DC508000 027F0002
        s_mov_b32 s7, 1                                            // 000000002468: BE870081
        s_mov_b32 s6, 0x3a83126f                                   // 00000000246C: BE8600FF 3A83126F
        s_waitcnt vmcnt(0)                                         // 000000002474: BF8C0F70

0000000000002478 <L0>:
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:16
;         temp[i] = in[idx] * (i + 1) * 0.001f;
        s_add_i32 s10, s9, 1                                       // 000000002478: 810A8109
        s_add_i32 s11, s7, 1                                       // 00000000247C: 810B8107
        v_cvt_f32_u32_e32 v5, s11                                  // 000000002480: 7E0A0C0B
        v_cvt_f32_u32_e32 v4, s10                                  // 000000002484: 7E080C0A
        v_mov_b32_e32 v3, s8                                       // 000000002488: 7E060208
        s_add_i32 s9, s9, 2                                        // 00000000248C: 81098209
        s_add_i32 s7, s7, 2                                        // 000000002490: 81078207
        s_add_i32 s8, s8, 8                                        // 000000002494: 81088808
        v_pk_mul_f32 v[4:5], v[2:3], v[4:5] op_sel_hi:[0,1]        // 000000002498: D3B14004 10020902
        s_cmpk_lg_i32 s8, 0x100                                    // 0000000024A0: B1880100
        v_pk_mul_f32 v[4:5], v[4:5], s[6:7] op_sel_hi:[1,0]        // 0000000024A4: D3B14004 08000D04
        buffer_store_dword v5, v3, s[0:3], 0 offen offset:4        // 0000000024AC: E0701004 80000503
        buffer_store_dword v4, v3, s[0:3], 0 offen                 // 0000000024B4: E0701000 80000403
        s_cbranch_scc1 L0                                          // 0000000024BC: BF85FFEE
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:21
;         sum += temp[i] * temp[(i + 1) % 64];
        buffer_load_dword v2, off, s[0:3], 0                       // 0000000024C0: E0500000 80000200
        buffer_load_dword v3, off, s[0:3], 0 offset:4              // 0000000024C8: E0500004 80000300
        buffer_load_dword v4, off, s[0:3], 0 offset:8              // 0000000024D0: E0500008 80000400
        buffer_load_dword v5, off, s[0:3], 0 offset:12             // 0000000024D8: E050000C 80000500
        buffer_load_dword v6, off, s[0:3], 0 offset:16             // 0000000024E0: E0500010 80000600
        buffer_load_dword v7, off, s[0:3], 0 offset:20             // 0000000024E8: E0500014 80000700
        buffer_load_dword v8, off, s[0:3], 0 offset:24             // 0000000024F0: E0500018 80000800
        buffer_load_dword v9, off, s[0:3], 0 offset:28             // 0000000024F8: E050001C 80000900
        buffer_load_dword v10, off, s[0:3], 0 offset:32            // 000000002500: E0500020 80000A00
        buffer_load_dword v11, off, s[0:3], 0 offset:36            // 000000002508: E0500024 80000B00
        buffer_load_dword v12, off, s[0:3], 0 offset:40            // 000000002510: E0500028 80000C00
        buffer_load_dword v13, off, s[0:3], 0 offset:44            // 000000002518: E050002C 80000D00
        buffer_load_dword v14, off, s[0:3], 0 offset:48            // 000000002520: E0500030 80000E00
        buffer_load_dword v15, off, s[0:3], 0 offset:52            // 000000002528: E0500034 80000F00
        buffer_load_dword v16, off, s[0:3], 0 offset:56            // 000000002530: E0500038 80001000
        buffer_load_dword v17, off, s[0:3], 0 offset:60            // 000000002538: E050003C 80001100
        buffer_load_dword v18, off, s[0:3], 0 offset:64            // 000000002540: E0500040 80001200
        buffer_load_dword v19, off, s[0:3], 0 offset:68            // 000000002548: E0500044 80001300
        buffer_load_dword v20, off, s[0:3], 0 offset:72            // 000000002550: E0500048 80001400
        buffer_load_dword v21, off, s[0:3], 0 offset:76            // 000000002558: E050004C 80001500
        buffer_load_dword v22, off, s[0:3], 0 offset:80            // 000000002560: E0500050 80001600
        buffer_load_dword v23, off, s[0:3], 0 offset:84            // 000000002568: E0500054 80001700
        buffer_load_dword v24, off, s[0:3], 0 offset:88            // 000000002570: E0500058 80001800
        buffer_load_dword v25, off, s[0:3], 0 offset:92            // 000000002578: E050005C 80001900
        buffer_load_dword v26, off, s[0:3], 0 offset:96            // 000000002580: E0500060 80001A00
        buffer_load_dword v27, off, s[0:3], 0 offset:100           // 000000002588: E0500064 80001B00
        buffer_load_dword v28, off, s[0:3], 0 offset:104           // 000000002590: E0500068 80001C00
        buffer_load_dword v29, off, s[0:3], 0 offset:108           // 000000002598: E050006C 80001D00
        buffer_load_dword v30, off, s[0:3], 0 offset:112           // 0000000025A0: E0500070 80001E00
        buffer_load_dword v31, off, s[0:3], 0 offset:116           // 0000000025A8: E0500074 80001F00
        buffer_load_dword v32, off, s[0:3], 0 offset:120           // 0000000025B0: E0500078 80002000
        buffer_load_dword v33, off, s[0:3], 0 offset:124           // 0000000025B8: E050007C 80002100
        buffer_load_dword v34, off, s[0:3], 0 offset:128           // 0000000025C0: E0500080 80002200
        buffer_load_dword v35, off, s[0:3], 0 offset:132           // 0000000025C8: E0500084 80002300
        buffer_load_dword v36, off, s[0:3], 0 offset:136           // 0000000025D0: E0500088 80002400
        buffer_load_dword v37, off, s[0:3], 0 offset:140           // 0000000025D8: E050008C 80002500
        buffer_load_dword v38, off, s[0:3], 0 offset:144           // 0000000025E0: E0500090 80002600
        buffer_load_dword v39, off, s[0:3], 0 offset:148           // 0000000025E8: E0500094 80002700
        buffer_load_dword v40, off, s[0:3], 0 offset:152           // 0000000025F0: E0500098 80002800
        buffer_load_dword v41, off, s[0:3], 0 offset:156           // 0000000025F8: E050009C 80002900
        buffer_load_dword v42, off, s[0:3], 0 offset:160           // 000000002600: E05000A0 80002A00
        buffer_load_dword v43, off, s[0:3], 0 offset:164           // 000000002608: E05000A4 80002B00
        buffer_load_dword v44, off, s[0:3], 0 offset:168           // 000000002610: E05000A8 80002C00
        buffer_load_dword v45, off, s[0:3], 0 offset:172           // 000000002618: E05000AC 80002D00
        buffer_load_dword v46, off, s[0:3], 0 offset:176           // 000000002620: E05000B0 80002E00
        buffer_load_dword v47, off, s[0:3], 0 offset:180           // 000000002628: E05000B4 80002F00
        buffer_load_dword v48, off, s[0:3], 0 offset:184           // 000000002630: E05000B8 80003000
        buffer_load_dword v49, off, s[0:3], 0 offset:188           // 000000002638: E05000BC 80003100
        buffer_load_dword v50, off, s[0:3], 0 offset:192           // 000000002640: E05000C0 80003200
        buffer_load_dword v51, off, s[0:3], 0 offset:196           // 000000002648: E05000C4 80003300
        buffer_load_dword v52, off, s[0:3], 0 offset:200           // 000000002650: E05000C8 80003400
        buffer_load_dword v53, off, s[0:3], 0 offset:204           // 000000002658: E05000CC 80003500
        buffer_load_dword v54, off, s[0:3], 0 offset:208           // 000000002660: E05000D0 80003600
        buffer_load_dword v55, off, s[0:3], 0 offset:212           // 000000002668: E05000D4 80003700
        buffer_load_dword v56, off, s[0:3], 0 offset:220           // 000000002670: E05000DC 80003800
        buffer_load_dword v57, off, s[0:3], 0 offset:216           // 000000002678: E05000D8 80003900
        buffer_load_dword v58, off, s[0:3], 0 offset:248           // 000000002680: E05000F8 80003A00
        s_waitcnt vmcnt(55)                                        // 000000002688: BF8CCF77
        v_fma_f32 v59, v2, v3, 0                                   // 00000000268C: D1CB003B 02020702
        s_waitcnt vmcnt(54)                                        // 000000002694: BF8CCF76
        v_fmac_f32_e32 v59, v3, v4                                 // 000000002698: 76760903
        s_waitcnt vmcnt(53)                                        // 00000000269C: BF8CCF75
        v_fmac_f32_e32 v59, v4, v5                                 // 0000000026A0: 76760B04
        s_waitcnt vmcnt(52)                                        // 0000000026A4: BF8CCF74
        v_fmac_f32_e32 v59, v5, v6                                 // 0000000026A8: 76760D05
        s_waitcnt vmcnt(51)                                        // 0000000026AC: BF8CCF73
        v_fmac_f32_e32 v59, v6, v7                                 // 0000000026B0: 76760F06
        s_waitcnt vmcnt(50)                                        // 0000000026B4: BF8CCF72
        v_fmac_f32_e32 v59, v7, v8                                 // 0000000026B8: 76761107
        buffer_load_dword v5, off, s[0:3], 0 offset:224            // 0000000026BC: E05000E0 80000500
        s_waitcnt vmcnt(50)                                        // 0000000026C4: BF8CCF72
        v_fmac_f32_e32 v59, v8, v9                                 // 0000000026C8: 76761308
        buffer_load_dword v3, off, s[0:3], 0 offset:240            // 0000000026CC: E05000F0 80000300
        buffer_load_dword v8, off, s[0:3], 0 offset:244            // 0000000026D4: E05000F4 80000800
        buffer_load_dword v4, off, s[0:3], 0 offset:232            // 0000000026DC: E05000E8 80000400
        s_waitcnt vmcnt(52)                                        // 0000000026E4: BF8CCF74
        v_fmac_f32_e32 v59, v9, v10                                // 0000000026E8: 76761509
        buffer_load_dword v9, off, s[0:3], 0 offset:252            // 0000000026EC: E05000FC 80000900
        buffer_load_dword v7, off, s[0:3], 0 offset:236            // 0000000026F4: E05000EC 80000700
        buffer_load_dword v6, off, s[0:3], 0 offset:228            // 0000000026FC: E05000E4 80000600
        s_waitcnt vmcnt(54)                                        // 000000002704: BF8CCF76
        v_fmac_f32_e32 v59, v10, v11                               // 000000002708: 7676170A
        s_waitcnt vmcnt(53)                                        // 00000000270C: BF8CCF75
        v_fmac_f32_e32 v59, v11, v12                               // 000000002710: 7676190B
        s_waitcnt vmcnt(52)                                        // 000000002714: BF8CCF74
        v_fmac_f32_e32 v59, v12, v13                               // 000000002718: 76761B0C
        s_waitcnt vmcnt(51)                                        // 00000000271C: BF8CCF73
        v_fmac_f32_e32 v59, v13, v14                               // 000000002720: 76761D0D
        s_waitcnt vmcnt(50)                                        // 000000002724: BF8CCF72
        v_fmac_f32_e32 v59, v14, v15                               // 000000002728: 76761F0E
        s_waitcnt vmcnt(49)                                        // 00000000272C: BF8CCF71
        v_fmac_f32_e32 v59, v15, v16                               // 000000002730: 7676210F
        s_waitcnt vmcnt(48)                                        // 000000002734: BF8CCF70
        v_fmac_f32_e32 v59, v16, v17                               // 000000002738: 76762310
        s_waitcnt vmcnt(47)                                        // 00000000273C: BF8C8F7F
        v_fmac_f32_e32 v59, v17, v18                               // 000000002740: 76762511
        s_waitcnt vmcnt(46)                                        // 000000002744: BF8C8F7E
        v_fmac_f32_e32 v59, v18, v19                               // 000000002748: 76762712
        s_waitcnt vmcnt(45)                                        // 00000000274C: BF8C8F7D
        v_fmac_f32_e32 v59, v19, v20                               // 000000002750: 76762913
        s_waitcnt vmcnt(44)                                        // 000000002754: BF8C8F7C
        v_fmac_f32_e32 v59, v20, v21                               // 000000002758: 76762B14
        s_waitcnt vmcnt(43)                                        // 00000000275C: BF8C8F7B
        v_fmac_f32_e32 v59, v21, v22                               // 000000002760: 76762D15
        s_waitcnt vmcnt(42)                                        // 000000002764: BF8C8F7A
        v_fmac_f32_e32 v59, v22, v23                               // 000000002768: 76762F16
        s_waitcnt vmcnt(41)                                        // 00000000276C: BF8C8F79
        v_fmac_f32_e32 v59, v23, v24                               // 000000002770: 76763117
        s_waitcnt vmcnt(40)                                        // 000000002774: BF8C8F78
        v_fmac_f32_e32 v59, v24, v25                               // 000000002778: 76763318
        s_waitcnt vmcnt(39)                                        // 00000000277C: BF8C8F77
        v_fmac_f32_e32 v59, v25, v26                               // 000000002780: 76763519
        s_waitcnt vmcnt(38)                                        // 000000002784: BF8C8F76
        v_fmac_f32_e32 v59, v26, v27                               // 000000002788: 7676371A
        s_waitcnt vmcnt(37)                                        // 00000000278C: BF8C8F75
        v_fmac_f32_e32 v59, v27, v28                               // 000000002790: 7676391B
        s_waitcnt vmcnt(36)                                        // 000000002794: BF8C8F74
        v_fmac_f32_e32 v59, v28, v29                               // 000000002798: 76763B1C
        s_waitcnt vmcnt(35)                                        // 00000000279C: BF8C8F73
        v_fmac_f32_e32 v59, v29, v30                               // 0000000027A0: 76763D1D
        s_waitcnt vmcnt(34)                                        // 0000000027A4: BF8C8F72
        v_fmac_f32_e32 v59, v30, v31                               // 0000000027A8: 76763F1E
        s_waitcnt vmcnt(33)                                        // 0000000027AC: BF8C8F71
        v_fmac_f32_e32 v59, v31, v32                               // 0000000027B0: 7676411F
        s_waitcnt vmcnt(32)                                        // 0000000027B4: BF8C8F70
        v_fmac_f32_e32 v59, v32, v33                               // 0000000027B8: 76764320
        s_waitcnt vmcnt(31)                                        // 0000000027BC: BF8C4F7F
        v_fmac_f32_e32 v59, v33, v34                               // 0000000027C0: 76764521
        s_waitcnt vmcnt(30)                                        // 0000000027C4: BF8C4F7E
        v_fmac_f32_e32 v59, v34, v35                               // 0000000027C8: 76764722
        s_waitcnt vmcnt(29)                                        // 0000000027CC: BF8C4F7D
        v_fmac_f32_e32 v59, v35, v36                               // 0000000027D0: 76764923
        s_waitcnt vmcnt(28)                                        // 0000000027D4: BF8C4F7C
        v_fmac_f32_e32 v59, v36, v37                               // 0000000027D8: 76764B24
        s_waitcnt vmcnt(27)                                        // 0000000027DC: BF8C4F7B
        v_fmac_f32_e32 v59, v37, v38                               // 0000000027E0: 76764D25
        s_waitcnt vmcnt(26)                                        // 0000000027E4: BF8C4F7A
        v_fmac_f32_e32 v59, v38, v39                               // 0000000027E8: 76764F26
        s_waitcnt vmcnt(25)                                        // 0000000027EC: BF8C4F79
        v_fmac_f32_e32 v59, v39, v40                               // 0000000027F0: 76765127
        s_waitcnt vmcnt(24)                                        // 0000000027F4: BF8C4F78
        v_fmac_f32_e32 v59, v40, v41                               // 0000000027F8: 76765328
        s_waitcnt vmcnt(23)                                        // 0000000027FC: BF8C4F77
        v_fmac_f32_e32 v59, v41, v42                               // 000000002800: 76765529
        s_waitcnt vmcnt(22)                                        // 000000002804: BF8C4F76
        v_fmac_f32_e32 v59, v42, v43                               // 000000002808: 7676572A
        s_waitcnt vmcnt(21)                                        // 00000000280C: BF8C4F75
        v_fmac_f32_e32 v59, v43, v44                               // 000000002810: 7676592B
        s_waitcnt vmcnt(20)                                        // 000000002814: BF8C4F74
        v_fmac_f32_e32 v59, v44, v45                               // 000000002818: 76765B2C
        s_waitcnt vmcnt(19)                                        // 00000000281C: BF8C4F73
        v_fmac_f32_e32 v59, v45, v46                               // 000000002820: 76765D2D
        s_waitcnt vmcnt(18)                                        // 000000002824: BF8C4F72
        v_fmac_f32_e32 v59, v46, v47                               // 000000002828: 76765F2E
        s_waitcnt vmcnt(17)                                        // 00000000282C: BF8C4F71
        v_fmac_f32_e32 v59, v47, v48                               // 000000002830: 7676612F
        s_waitcnt vmcnt(16)                                        // 000000002834: BF8C4F70
        v_fmac_f32_e32 v59, v48, v49                               // 000000002838: 76766330
        s_waitcnt vmcnt(15)                                        // 00000000283C: BF8C0F7F
        v_fmac_f32_e32 v59, v49, v50                               // 000000002840: 76766531
        s_waitcnt vmcnt(14)                                        // 000000002844: BF8C0F7E
        v_fmac_f32_e32 v59, v50, v51                               // 000000002848: 76766732
        s_waitcnt vmcnt(13)                                        // 00000000284C: BF8C0F7D
        v_fmac_f32_e32 v59, v51, v52                               // 000000002850: 76766933
        s_waitcnt vmcnt(12)                                        // 000000002854: BF8C0F7C
        v_fmac_f32_e32 v59, v52, v53                               // 000000002858: 76766B34
        s_waitcnt vmcnt(11)                                        // 00000000285C: BF8C0F7B
        v_fmac_f32_e32 v59, v53, v54                               // 000000002860: 76766D35
        s_waitcnt vmcnt(10)                                        // 000000002864: BF8C0F7A
        v_fmac_f32_e32 v59, v54, v55                               // 000000002868: 76766F36
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:24
;     out[idx] = sum;
        v_mov_b32_e32 v10, s5                                      // 00000000286C: 7E140205
        v_add_co_u32_e32 v0, vcc, s4, v0                           // 000000002870: 32000004
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:21
;         sum += temp[i] * temp[(i + 1) % 64];
        s_waitcnt vmcnt(8)                                         // 000000002874: BF8C0F78
        v_fmac_f32_e32 v59, v55, v57                               // 000000002878: 76767337
        v_fmac_f32_e32 v59, v57, v56                               // 00000000287C: 76767139
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:24
;     out[idx] = sum;
        v_addc_co_u32_e32 v1, vcc, v10, v1, vcc                    // 000000002880: 3802030A
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:21
;         sum += temp[i] * temp[(i + 1) % 64];
        s_waitcnt vmcnt(6)                                         // 000000002884: BF8C0F76
        v_fmac_f32_e32 v59, v56, v5                                // 000000002888: 76760B38
        s_waitcnt vmcnt(0)                                         // 00000000288C: BF8C0F70
        v_fmac_f32_e32 v59, v5, v6                                 // 000000002890: 76760D05
        v_fmac_f32_e32 v59, v6, v4                                 // 000000002894: 76760906
        v_fmac_f32_e32 v59, v4, v7                                 // 000000002898: 76760F04
        v_fmac_f32_e32 v59, v7, v3                                 // 00000000289C: 76760707
        v_fmac_f32_e32 v59, v3, v8                                 // 0000000028A0: 76761103
        v_fmac_f32_e32 v59, v8, v58                                // 0000000028A4: 76767508
        v_fmac_f32_e32 v59, v58, v9                                // 0000000028A8: 7676133A
        v_fmac_f32_e32 v59, v9, v2                                 // 0000000028AC: 76760509
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:24
;     out[idx] = sum;
        global_store_dword v[0:1], v59, off                        // 0000000028B0: DC708000 007F3B00

00000000000028b8 <L1>:
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:25
; }
        s_endpgm                                                   // 0000000028B8: BF810000
        s_nop 0                                                    // 0000000028BC: BF800000
        s_nop 0                                                    // 0000000028C0: BF800000
        s_nop 0                                                    // 0000000028C4: BF800000
        s_nop 0                                                    // 0000000028C8: BF800000
        s_nop 0                                                    // 0000000028CC: BF800000
        s_nop 0                                                    // 0000000028D0: BF800000
        s_nop 0                                                    // 0000000028D4: BF800000
        s_nop 0                                                    // 0000000028D8: BF800000
        s_nop 0                                                    // 0000000028DC: BF800000
        s_nop 0                                                    // 0000000028E0: BF800000
        s_nop 0                                                    // 0000000028E4: BF800000
        s_nop 0                                                    // 0000000028E8: BF800000
        s_nop 0                                                    // 0000000028EC: BF800000
        s_nop 0                                                    // 0000000028F0: BF800000
        s_nop 0                                                    // 0000000028F4: BF800000
        s_nop 0                                                    // 0000000028F8: BF800000
        s_nop 0                                                    // 0000000028FC: BF800000

0000000000002900 <_Z14spillingKernelPfS_S_>:
; _Z14spillingKernelPfS_S_():
; /opt/rocm-7.1.0/lib/llvm/bin/../../../include/hip/amd_detail/amd_hip_runtime.h:264
; __DEVICE__ unsigned int __hip_get_block_dim_x() { return __ockl_get_local_size(0); }
        s_load_dword s6, s[4:5], 0x24                              // 000000002900: C0020182 00000024
        s_add_u32 s0, s0, s9                                       // 000000002908: 80000900
        s_addc_u32 s1, s1, 0                                       // 00000000290C: 82018001
        s_waitcnt lgkmcnt(0)                                       // 000000002910: BF8CC07F
        s_and_b32 s6, s6, 0xffff                                   // 000000002914: 8606FF06 0000FFFF
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:31
;     int idx = threadIdx.x + blockIdx.x * blockDim.x;
        s_mul_i32 s8, s8, s6                                       // 00000000291C: 92080608
        v_add_u32_e32 v0, s8, v0                                   // 000000002920: 68000008
        s_movk_i32 s6, 0x400                                       // 000000002924: B0060400
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:32
;     if (idx >= N) return;
        v_cmp_gt_i32_e32 vcc, s6, v0                               // 000000002928: 7D880006
        s_and_saveexec_b64 s[6:7], vcc                             // 00000000292C: BE86206A
        s_cbranch_execz L3                                         // 000000002930: BF880129
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:29
;     float temp[64]; // Adjust size based on register pressure
        s_load_dwordx4 s[8:11], s[4:5], 0x0                        // 000000002934: C00A0202 00000000
        s_load_dwordx2 s[6:7], s[4:5], 0x10                        // 00000000293C: C0060182 00000010
        v_ashrrev_i32_e32 v1, 31, v0                               // 000000002944: 2202009F
        v_lshlrev_b64 v[0:1], 2, v[0:1]                            // 000000002948: D28F0000 00020082
        s_mov_b32 s5, 1                                            // 000000002950: BE850081
        s_waitcnt lgkmcnt(0)                                       // 000000002954: BF8CC07F
        v_mov_b32_e32 v3, s11                                      // 000000002958: 7E06020B
        v_add_co_u32_e32 v2, vcc, s10, v0                          // 00000000295C: 3204000A
        v_addc_co_u32_e32 v3, vcc, v3, v1, vcc                     // 000000002960: 38060303
        global_load_dword v4, v[2:3], off                          // 000000002964: DC508000 047F0002
        v_mov_b32_e32 v3, s7                                       // 00000000296C: 7E060207
        v_add_co_u32_e32 v2, vcc, s6, v0                           // 000000002970: 32040006
        v_addc_co_u32_e32 v3, vcc, v3, v1, vcc                     // 000000002974: 38060303
        global_load_dword v2, v[2:3], off                          // 000000002978: DC508000 027F0002
        s_mov_b32 s6, 0                                            // 000000002980: BE860080
        s_mov_b32 s4, 0x3a83126f                                   // 000000002984: BE8400FF 3A83126F
        s_mov_b32 s7, 0                                            // 00000000298C: BE870080
        s_waitcnt vmcnt(0)                                         // 000000002990: BF8C0F70
        v_mul_f32_e32 v2, v4, v2                                   // 000000002994: 0A040504

0000000000002998 <L2>:
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:37
;         temp[i] = in[idx] * in2[idx] * (i + 1) * 0.001f;
        s_add_i32 s10, s7, 1                                       // 000000002998: 810A8107
        s_add_i32 s11, s5, 1                                       // 00000000299C: 810B8105
        v_cvt_f32_u32_e32 v5, s11                                  // 0000000029A0: 7E0A0C0B
        v_cvt_f32_u32_e32 v4, s10                                  // 0000000029A4: 7E080C0A
        v_mov_b32_e32 v3, s6                                       // 0000000029A8: 7E060206
        s_add_i32 s7, s7, 2                                        // 0000000029AC: 81078207
        s_add_i32 s5, s5, 2                                        // 0000000029B0: 81058205
        s_add_i32 s6, s6, 8                                        // 0000000029B4: 81068806
        v_pk_mul_f32 v[4:5], v[2:3], v[4:5] op_sel_hi:[0,1]        // 0000000029B8: D3B14004 10020902
        s_cmpk_lg_i32 s6, 0x100                                    // 0000000029C0: B1860100
        v_pk_mul_f32 v[4:5], v[4:5], s[4:5] op_sel_hi:[1,0]        // 0000000029C4: D3B14004 08000904
        buffer_store_dword v5, v3, s[0:3], 0 offen offset:4        // 0000000029CC: E0701004 80000503
        buffer_store_dword v4, v3, s[0:3], 0 offen                 // 0000000029D4: E0701000 80000403
        s_cbranch_scc1 L2                                          // 0000000029DC: BF85FFEE
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:42
;         sum += temp[i] * temp[(i + 1) % 64];
        buffer_load_dword v2, off, s[0:3], 0                       // 0000000029E0: E0500000 80000200
        buffer_load_dword v3, off, s[0:3], 0 offset:4              // 0000000029E8: E0500004 80000300
        buffer_load_dword v4, off, s[0:3], 0 offset:8              // 0000000029F0: E0500008 80000400
        buffer_load_dword v5, off, s[0:3], 0 offset:12             // 0000000029F8: E050000C 80000500
        buffer_load_dword v6, off, s[0:3], 0 offset:16             // 000000002A00: E0500010 80000600
        buffer_load_dword v7, off, s[0:3], 0 offset:20             // 000000002A08: E0500014 80000700
        buffer_load_dword v8, off, s[0:3], 0 offset:24             // 000000002A10: E0500018 80000800
        buffer_load_dword v9, off, s[0:3], 0 offset:28             // 000000002A18: E050001C 80000900
        buffer_load_dword v10, off, s[0:3], 0 offset:32            // 000000002A20: E0500020 80000A00
        buffer_load_dword v11, off, s[0:3], 0 offset:36            // 000000002A28: E0500024 80000B00
        buffer_load_dword v12, off, s[0:3], 0 offset:40            // 000000002A30: E0500028 80000C00
        buffer_load_dword v13, off, s[0:3], 0 offset:44            // 000000002A38: E050002C 80000D00
        buffer_load_dword v14, off, s[0:3], 0 offset:48            // 000000002A40: E0500030 80000E00
        buffer_load_dword v15, off, s[0:3], 0 offset:52            // 000000002A48: E0500034 80000F00
        buffer_load_dword v16, off, s[0:3], 0 offset:56            // 000000002A50: E0500038 80001000
        buffer_load_dword v17, off, s[0:3], 0 offset:60            // 000000002A58: E050003C 80001100
        buffer_load_dword v18, off, s[0:3], 0 offset:64            // 000000002A60: E0500040 80001200
        buffer_load_dword v19, off, s[0:3], 0 offset:68            // 000000002A68: E0500044 80001300
        buffer_load_dword v20, off, s[0:3], 0 offset:72            // 000000002A70: E0500048 80001400
        buffer_load_dword v21, off, s[0:3], 0 offset:76            // 000000002A78: E050004C 80001500
        buffer_load_dword v22, off, s[0:3], 0 offset:80            // 000000002A80: E0500050 80001600
        buffer_load_dword v23, off, s[0:3], 0 offset:84            // 000000002A88: E0500054 80001700
        buffer_load_dword v24, off, s[0:3], 0 offset:88            // 000000002A90: E0500058 80001800
        buffer_load_dword v25, off, s[0:3], 0 offset:92            // 000000002A98: E050005C 80001900
        buffer_load_dword v26, off, s[0:3], 0 offset:96            // 000000002AA0: E0500060 80001A00
        buffer_load_dword v27, off, s[0:3], 0 offset:100           // 000000002AA8: E0500064 80001B00
        buffer_load_dword v28, off, s[0:3], 0 offset:104           // 000000002AB0: E0500068 80001C00
        buffer_load_dword v29, off, s[0:3], 0 offset:108           // 000000002AB8: E050006C 80001D00
        buffer_load_dword v30, off, s[0:3], 0 offset:112           // 000000002AC0: E0500070 80001E00
        buffer_load_dword v31, off, s[0:3], 0 offset:116           // 000000002AC8: E0500074 80001F00
        buffer_load_dword v32, off, s[0:3], 0 offset:120           // 000000002AD0: E0500078 80002000
        buffer_load_dword v33, off, s[0:3], 0 offset:124           // 000000002AD8: E050007C 80002100
        buffer_load_dword v34, off, s[0:3], 0 offset:128           // 000000002AE0: E0500080 80002200
        buffer_load_dword v35, off, s[0:3], 0 offset:132           // 000000002AE8: E0500084 80002300
        buffer_load_dword v36, off, s[0:3], 0 offset:136           // 000000002AF0: E0500088 80002400
        buffer_load_dword v37, off, s[0:3], 0 offset:140           // 000000002AF8: E050008C 80002500
        buffer_load_dword v38, off, s[0:3], 0 offset:144           // 000000002B00: E0500090 80002600
        buffer_load_dword v39, off, s[0:3], 0 offset:148           // 000000002B08: E0500094 80002700
        buffer_load_dword v40, off, s[0:3], 0 offset:152           // 000000002B10: E0500098 80002800
        buffer_load_dword v41, off, s[0:3], 0 offset:156           // 000000002B18: E050009C 80002900
        buffer_load_dword v42, off, s[0:3], 0 offset:160           // 000000002B20: E05000A0 80002A00
        buffer_load_dword v43, off, s[0:3], 0 offset:164           // 000000002B28: E05000A4 80002B00
        buffer_load_dword v44, off, s[0:3], 0 offset:168           // 000000002B30: E05000A8 80002C00
        buffer_load_dword v45, off, s[0:3], 0 offset:172           // 000000002B38: E05000AC 80002D00
        buffer_load_dword v46, off, s[0:3], 0 offset:176           // 000000002B40: E05000B0 80002E00
        buffer_load_dword v47, off, s[0:3], 0 offset:180           // 000000002B48: E05000B4 80002F00
        buffer_load_dword v48, off, s[0:3], 0 offset:184           // 000000002B50: E05000B8 80003000
        buffer_load_dword v49, off, s[0:3], 0 offset:188           // 000000002B58: E05000BC 80003100
        buffer_load_dword v50, off, s[0:3], 0 offset:192           // 000000002B60: E05000C0 80003200
        buffer_load_dword v51, off, s[0:3], 0 offset:196           // 000000002B68: E05000C4 80003300
        buffer_load_dword v52, off, s[0:3], 0 offset:200           // 000000002B70: E05000C8 80003400
        buffer_load_dword v53, off, s[0:3], 0 offset:204           // 000000002B78: E05000CC 80003500
        buffer_load_dword v54, off, s[0:3], 0 offset:208           // 000000002B80: E05000D0 80003600
        buffer_load_dword v55, off, s[0:3], 0 offset:212           // 000000002B88: E05000D4 80003700
        buffer_load_dword v56, off, s[0:3], 0 offset:220           // 000000002B90: E05000DC 80003800
        buffer_load_dword v57, off, s[0:3], 0 offset:216           // 000000002B98: E05000D8 80003900
        buffer_load_dword v58, off, s[0:3], 0 offset:248           // 000000002BA0: E05000F8 80003A00
        s_waitcnt vmcnt(55)                                        // 000000002BA8: BF8CCF77
        v_fma_f32 v59, v2, v3, 0                                   // 000000002BAC: D1CB003B 02020702
        s_waitcnt vmcnt(54)                                        // 000000002BB4: BF8CCF76
        v_fmac_f32_e32 v59, v3, v4                                 // 000000002BB8: 76760903
        s_waitcnt vmcnt(53)                                        // 000000002BBC: BF8CCF75
        v_fmac_f32_e32 v59, v4, v5                                 // 000000002BC0: 76760B04
        s_waitcnt vmcnt(52)                                        // 000000002BC4: BF8CCF74
        v_fmac_f32_e32 v59, v5, v6                                 // 000000002BC8: 76760D05
        s_waitcnt vmcnt(51)                                        // 000000002BCC: BF8CCF73
        v_fmac_f32_e32 v59, v6, v7                                 // 000000002BD0: 76760F06
        s_waitcnt vmcnt(50)                                        // 000000002BD4: BF8CCF72
        v_fmac_f32_e32 v59, v7, v8                                 // 000000002BD8: 76761107
        buffer_load_dword v5, off, s[0:3], 0 offset:224            // 000000002BDC: E05000E0 80000500
        s_waitcnt vmcnt(50)                                        // 000000002BE4: BF8CCF72
        v_fmac_f32_e32 v59, v8, v9                                 // 000000002BE8: 76761308
        buffer_load_dword v3, off, s[0:3], 0 offset:240            // 000000002BEC: E05000F0 80000300
        buffer_load_dword v8, off, s[0:3], 0 offset:244            // 000000002BF4: E05000F4 80000800
        buffer_load_dword v4, off, s[0:3], 0 offset:232            // 000000002BFC: E05000E8 80000400
        s_waitcnt vmcnt(52)                                        // 000000002C04: BF8CCF74
        v_fmac_f32_e32 v59, v9, v10                                // 000000002C08: 76761509
        buffer_load_dword v9, off, s[0:3], 0 offset:252            // 000000002C0C: E05000FC 80000900
        buffer_load_dword v7, off, s[0:3], 0 offset:236            // 000000002C14: E05000EC 80000700
        buffer_load_dword v6, off, s[0:3], 0 offset:228            // 000000002C1C: E05000E4 80000600
        s_waitcnt vmcnt(54)                                        // 000000002C24: BF8CCF76
        v_fmac_f32_e32 v59, v10, v11                               // 000000002C28: 7676170A
        s_waitcnt vmcnt(53)                                        // 000000002C2C: BF8CCF75
        v_fmac_f32_e32 v59, v11, v12                               // 000000002C30: 7676190B
        s_waitcnt vmcnt(52)                                        // 000000002C34: BF8CCF74
        v_fmac_f32_e32 v59, v12, v13                               // 000000002C38: 76761B0C
        s_waitcnt vmcnt(51)                                        // 000000002C3C: BF8CCF73
        v_fmac_f32_e32 v59, v13, v14                               // 000000002C40: 76761D0D
        s_waitcnt vmcnt(50)                                        // 000000002C44: BF8CCF72
        v_fmac_f32_e32 v59, v14, v15                               // 000000002C48: 76761F0E
        s_waitcnt vmcnt(49)                                        // 000000002C4C: BF8CCF71
        v_fmac_f32_e32 v59, v15, v16                               // 000000002C50: 7676210F
        s_waitcnt vmcnt(48)                                        // 000000002C54: BF8CCF70
        v_fmac_f32_e32 v59, v16, v17                               // 000000002C58: 76762310
        s_waitcnt vmcnt(47)                                        // 000000002C5C: BF8C8F7F
        v_fmac_f32_e32 v59, v17, v18                               // 000000002C60: 76762511
        s_waitcnt vmcnt(46)                                        // 000000002C64: BF8C8F7E
        v_fmac_f32_e32 v59, v18, v19                               // 000000002C68: 76762712
        s_waitcnt vmcnt(45)                                        // 000000002C6C: BF8C8F7D
        v_fmac_f32_e32 v59, v19, v20                               // 000000002C70: 76762913
        s_waitcnt vmcnt(44)                                        // 000000002C74: BF8C8F7C
        v_fmac_f32_e32 v59, v20, v21                               // 000000002C78: 76762B14
        s_waitcnt vmcnt(43)                                        // 000000002C7C: BF8C8F7B
        v_fmac_f32_e32 v59, v21, v22                               // 000000002C80: 76762D15
        s_waitcnt vmcnt(42)                                        // 000000002C84: BF8C8F7A
        v_fmac_f32_e32 v59, v22, v23                               // 000000002C88: 76762F16
        s_waitcnt vmcnt(41)                                        // 000000002C8C: BF8C8F79
        v_fmac_f32_e32 v59, v23, v24                               // 000000002C90: 76763117
        s_waitcnt vmcnt(40)                                        // 000000002C94: BF8C8F78
        v_fmac_f32_e32 v59, v24, v25                               // 000000002C98: 76763318
        s_waitcnt vmcnt(39)                                        // 000000002C9C: BF8C8F77
        v_fmac_f32_e32 v59, v25, v26                               // 000000002CA0: 76763519
        s_waitcnt vmcnt(38)                                        // 000000002CA4: BF8C8F76
        v_fmac_f32_e32 v59, v26, v27                               // 000000002CA8: 7676371A
        s_waitcnt vmcnt(37)                                        // 000000002CAC: BF8C8F75
        v_fmac_f32_e32 v59, v27, v28                               // 000000002CB0: 7676391B
        s_waitcnt vmcnt(36)                                        // 000000002CB4: BF8C8F74
        v_fmac_f32_e32 v59, v28, v29                               // 000000002CB8: 76763B1C
        s_waitcnt vmcnt(35)                                        // 000000002CBC: BF8C8F73
        v_fmac_f32_e32 v59, v29, v30                               // 000000002CC0: 76763D1D
        s_waitcnt vmcnt(34)                                        // 000000002CC4: BF8C8F72
        v_fmac_f32_e32 v59, v30, v31                               // 000000002CC8: 76763F1E
        s_waitcnt vmcnt(33)                                        // 000000002CCC: BF8C8F71
        v_fmac_f32_e32 v59, v31, v32                               // 000000002CD0: 7676411F
        s_waitcnt vmcnt(32)                                        // 000000002CD4: BF8C8F70
        v_fmac_f32_e32 v59, v32, v33                               // 000000002CD8: 76764320
        s_waitcnt vmcnt(31)                                        // 000000002CDC: BF8C4F7F
        v_fmac_f32_e32 v59, v33, v34                               // 000000002CE0: 76764521
        s_waitcnt vmcnt(30)                                        // 000000002CE4: BF8C4F7E
        v_fmac_f32_e32 v59, v34, v35                               // 000000002CE8: 76764722
        s_waitcnt vmcnt(29)                                        // 000000002CEC: BF8C4F7D
        v_fmac_f32_e32 v59, v35, v36                               // 000000002CF0: 76764923
        s_waitcnt vmcnt(28)                                        // 000000002CF4: BF8C4F7C
        v_fmac_f32_e32 v59, v36, v37                               // 000000002CF8: 76764B24
        s_waitcnt vmcnt(27)                                        // 000000002CFC: BF8C4F7B
        v_fmac_f32_e32 v59, v37, v38                               // 000000002D00: 76764D25
        s_waitcnt vmcnt(26)                                        // 000000002D04: BF8C4F7A
        v_fmac_f32_e32 v59, v38, v39                               // 000000002D08: 76764F26
        s_waitcnt vmcnt(25)                                        // 000000002D0C: BF8C4F79
        v_fmac_f32_e32 v59, v39, v40                               // 000000002D10: 76765127
        s_waitcnt vmcnt(24)                                        // 000000002D14: BF8C4F78
        v_fmac_f32_e32 v59, v40, v41                               // 000000002D18: 76765328
        s_waitcnt vmcnt(23)                                        // 000000002D1C: BF8C4F77
        v_fmac_f32_e32 v59, v41, v42                               // 000000002D20: 76765529
        s_waitcnt vmcnt(22)                                        // 000000002D24: BF8C4F76
        v_fmac_f32_e32 v59, v42, v43                               // 000000002D28: 7676572A
        s_waitcnt vmcnt(21)                                        // 000000002D2C: BF8C4F75
        v_fmac_f32_e32 v59, v43, v44                               // 000000002D30: 7676592B
        s_waitcnt vmcnt(20)                                        // 000000002D34: BF8C4F74
        v_fmac_f32_e32 v59, v44, v45                               // 000000002D38: 76765B2C
        s_waitcnt vmcnt(19)                                        // 000000002D3C: BF8C4F73
        v_fmac_f32_e32 v59, v45, v46                               // 000000002D40: 76765D2D
        s_waitcnt vmcnt(18)                                        // 000000002D44: BF8C4F72
        v_fmac_f32_e32 v59, v46, v47                               // 000000002D48: 76765F2E
        s_waitcnt vmcnt(17)                                        // 000000002D4C: BF8C4F71
        v_fmac_f32_e32 v59, v47, v48                               // 000000002D50: 7676612F
        s_waitcnt vmcnt(16)                                        // 000000002D54: BF8C4F70
        v_fmac_f32_e32 v59, v48, v49                               // 000000002D58: 76766330
        s_waitcnt vmcnt(15)                                        // 000000002D5C: BF8C0F7F
        v_fmac_f32_e32 v59, v49, v50                               // 000000002D60: 76766531
        s_waitcnt vmcnt(14)                                        // 000000002D64: BF8C0F7E
        v_fmac_f32_e32 v59, v50, v51                               // 000000002D68: 76766732
        s_waitcnt vmcnt(13)                                        // 000000002D6C: BF8C0F7D
        v_fmac_f32_e32 v59, v51, v52                               // 000000002D70: 76766933
        s_waitcnt vmcnt(12)                                        // 000000002D74: BF8C0F7C
        v_fmac_f32_e32 v59, v52, v53                               // 000000002D78: 76766B34
        s_waitcnt vmcnt(11)                                        // 000000002D7C: BF8C0F7B
        v_fmac_f32_e32 v59, v53, v54                               // 000000002D80: 76766D35
        s_waitcnt vmcnt(10)                                        // 000000002D84: BF8C0F7A
        v_fmac_f32_e32 v59, v54, v55                               // 000000002D88: 76766F36
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:45
;     out[idx] = sum;
        v_mov_b32_e32 v10, s9                                      // 000000002D8C: 7E140209
        v_add_co_u32_e32 v0, vcc, s8, v0                           // 000000002D90: 32000008
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:42
;         sum += temp[i] * temp[(i + 1) % 64];
        s_waitcnt vmcnt(8)                                         // 000000002D94: BF8C0F78
        v_fmac_f32_e32 v59, v55, v57                               // 000000002D98: 76767337
        v_fmac_f32_e32 v59, v57, v56                               // 000000002D9C: 76767139
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:45
;     out[idx] = sum;
        v_addc_co_u32_e32 v1, vcc, v10, v1, vcc                    // 000000002DA0: 3802030A
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:42
;         sum += temp[i] * temp[(i + 1) % 64];
        s_waitcnt vmcnt(6)                                         // 000000002DA4: BF8C0F76
        v_fmac_f32_e32 v59, v56, v5                                // 000000002DA8: 76760B38
        s_waitcnt vmcnt(0)                                         // 000000002DAC: BF8C0F70
        v_fmac_f32_e32 v59, v5, v6                                 // 000000002DB0: 76760D05
        v_fmac_f32_e32 v59, v6, v4                                 // 000000002DB4: 76760906
        v_fmac_f32_e32 v59, v4, v7                                 // 000000002DB8: 76760F04
        v_fmac_f32_e32 v59, v7, v3                                 // 000000002DBC: 76760707
        v_fmac_f32_e32 v59, v3, v8                                 // 000000002DC0: 76761103
        v_fmac_f32_e32 v59, v8, v58                                // 000000002DC4: 76767508
        v_fmac_f32_e32 v59, v58, v9                                // 000000002DC8: 7676133A
        v_fmac_f32_e32 v59, v9, v2                                 // 000000002DCC: 76760509
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:45
;     out[idx] = sum;
        global_store_dword v[0:1], v59, off                        // 000000002DD0: DC708000 007F3B00

0000000000002dd8 <L3>:
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:46
; }
        s_endpgm                                                   // 000000002DD8: BF810000
        s_nop 0                                                    // 000000002DDC: BF800000
        s_nop 0                                                    // 000000002DE0: BF800000
        s_nop 0                                                    // 000000002DE4: BF800000
        s_nop 0                                                    // 000000002DE8: BF800000
        s_nop 0                                                    // 000000002DEC: BF800000
        s_nop 0                                                    // 000000002DF0: BF800000
        s_nop 0                                                    // 000000002DF4: BF800000
        s_nop 0                                                    // 000000002DF8: BF800000
        s_nop 0                                                    // 000000002DFC: BF800000

0000000000002e00 <_Z12vectorKernelPfS_>:
; _Z12vectorKernelPfS_():
; /opt/rocm-7.1.0/lib/llvm/bin/../../../include/hip/amd_detail/amd_hip_runtime.h:264
; __DEVICE__ unsigned int __hip_get_block_dim_x() { return __ockl_get_local_size(0); }
        s_load_dword s6, s[4:5], 0x1c                              // 000000002E00: C0020182 0000001C
        s_add_u32 s0, s0, s9                                       // 000000002E08: 80000900
        s_addc_u32 s1, s1, 0                                       // 000000002E0C: 82018001
        s_waitcnt lgkmcnt(0)                                       // 000000002E10: BF8CC07F
        s_and_b32 s6, s6, 0xffff                                   // 000000002E14: 8606FF06 0000FFFF
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:52
;     int idx = threadIdx.x + blockIdx.x * blockDim.x;
        s_mul_i32 s8, s8, s6                                       // 000000002E1C: 92080608
        v_add_u32_e32 v0, s8, v0                                   // 000000002E20: 68000008
        s_movk_i32 s6, 0x400                                       // 000000002E24: B0060400
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:53
;     if (idx >= N) return;
        v_cmp_gt_i32_e32 vcc, s6, v0                               // 000000002E28: 7D880006
        s_and_saveexec_b64 s[6:7], vcc                             // 000000002E2C: BE86206A
        s_cbranch_execz L10                                        // 000000002E30: BF8801DE
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:50
;     float temp[72]; // Slightly larger for different spilling pattern
        s_load_dwordx4 s[12:15], s[4:5], 0x0                       // 000000002E34: C00A0302 00000000
        v_ashrrev_i32_e32 v1, 31, v0                               // 000000002E3C: 2202009F
        v_lshlrev_b64 v[0:1], 2, v[0:1]                            // 000000002E40: D28F0000 00020082
        s_mov_b32 s27, 0                                           // 000000002E48: BE9B0080
        s_mov_b32 s16, 0xfe5163ab                                  // 000000002E4C: BE9000FF FE5163AB
        s_waitcnt lgkmcnt(0)                                       // 000000002E54: BF8CC07F
        v_mov_b32_e32 v3, s15                                      // 000000002E58: 7E06020F
        v_add_co_u32_e32 v2, vcc, s14, v0                          // 000000002E5C: 3204000E
        v_addc_co_u32_e32 v3, vcc, v3, v1, vcc                     // 000000002E60: 38060303
        global_load_dword v4, v[2:3], off                          // 000000002E64: DC508000 047F0002
        s_mov_b32 s14, 0                                           // 000000002E6C: BE8E0080
        s_brev_b32 s15, 18                                         // 000000002E70: BE8F0892
        s_mov_b32 s17, 0x3c439041                                  // 000000002E74: BE9100FF 3C439041
        s_mov_b32 s18, 0xdb629599                                  // 000000002E7C: BE9200FF DB629599
        s_mov_b32 s19, 0xf534ddc0                                  // 000000002E84: BE9300FF F534DDC0
        s_mov_b32 s20, 0xfc2757d1                                  // 000000002E8C: BE9400FF FC2757D1
        s_mov_b32 s21, 0x4e441529                                  // 000000002E94: BE9500FF 4E441529
        s_mov_b32 s22, 0xa2f9836e                                  // 000000002E9C: BE9600FF A2F9836E
        s_mov_b32 s23, 0x3fc90fda                                  // 000000002EA4: BE9700FF 3FC90FDA
        s_mov_b32 s24, 0x3f22f983                                  // 000000002EAC: BE9800FF 3F22F983
        s_mov_b32 s25, 0xbfc90fda                                  // 000000002EB4: BE9900FF BFC90FDA
        v_mov_b32_e32 v5, 0xbe2aaa9d                               // 000000002EBC: 7E0A02FF BE2AAA9D
        v_mov_b32_e32 v6, 0x3d2aabf7                               // 000000002EC4: 7E0C02FF 3D2AABF7
        v_mov_b32_e32 v7, 0xbf000004                               // 000000002ECC: 7E0E02FF BF000004
        s_movk_i32 s26, 0x1f8                                      // 000000002ED4: B01A01F8
        v_mov_b32_e32 v3, 0                                        // 000000002ED8: 7E060280
        v_not_b32_e32 v8, 63                                       // 000000002EDC: 7E1056BF
        v_not_b32_e32 v9, 31                                       // 000000002EE0: 7E12569F
        v_mov_b32_e32 v10, 0x7fc00000                              // 000000002EE4: 7E1402FF 7FC00000
        s_waitcnt vmcnt(0)                                         // 000000002EEC: BF8C0F70
        s_branch L5                                                // 000000002EF0: BF82002E

0000000000002ef4 <L4>:
        s_or_b64 exec, exec, s[4:5]                                // 000000002EF4: 87FE047E
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:59
;         temp[i] = x * x * 0.5f + x * 1.2f + 3.4f * sinf(x * 0.1f);
        v_mul_f32_e32 v15, v13, v13                                // 000000002EF8: 0A1E1B0D
        v_mul_f32_e32 v15, 0.5, v15                                // 000000002EFC: 0A1E1EF0
        v_fmac_f32_e32 v15, 0x3f99999a, v13                        // 000000002F00: 761E1AFF 3F99999A
; /opt/rocm-7.1.0/lib/llvm/lib/clang/20/include/__clang_hip_math.h:707
; float sinf(float __x) { return __FAST_OR_SLOW(__sinf, __ocml_sin_f32)(__x); }
        v_mul_f32_e32 v13, v14, v14                                // 000000002F08: 0A1A1D0E
        v_mov_b32_e32 v16, 0x3c0881c4                              // 000000002F0C: 7E2002FF 3C0881C4
        v_fmac_f32_e32 v16, 0xb94c1982, v13                        // 000000002F14: 76201AFF B94C1982
        v_fma_f32 v16, v13, v16, v5                                // 000000002F1C: D1CB0010 0416210D
        v_mul_f32_e32 v16, v13, v16                                // 000000002F24: 0A20210D
        v_fmac_f32_e32 v14, v14, v16                               // 000000002F28: 761C210E
        v_mov_b32_e32 v16, 0xbab64f3b                              // 000000002F2C: 7E2002FF BAB64F3B
        v_fmac_f32_e32 v16, 0x37d75334, v13                        // 000000002F34: 76201AFF 37D75334
        v_fma_f32 v16, v13, v16, v6                                // 000000002F3C: D1CB0010 041A210D
        v_fma_f32 v16, v13, v16, v7                                // 000000002F44: D1CB0010 041E210D
        v_fma_f32 v13, v13, v16, 1.0                               // 000000002F4C: D1CB000D 03CA210D
        v_and_b32_e32 v16, 1, v2                                   // 000000002F54: 26200481
        v_lshlrev_b32_e32 v2, 30, v2                               // 000000002F58: 2404049E
        v_cmp_eq_u32_e32 vcc, 0, v16                               // 000000002F5C: 7D942080
        v_and_b32_e32 v2, 0x80000000, v2                           // 000000002F60: 260404FF 80000000
        v_xor_b32_e32 v12, v12, v11                                // 000000002F68: 2A18170C
        v_cndmask_b32_e32 v13, v13, v14, vcc                       // 000000002F6C: 001A1D0D
        v_xor_b32_e32 v2, v12, v2                                  // 000000002F70: 2A04050C
        v_xor_b32_e32 v2, v2, v13                                  // 000000002F74: 2A041B02
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:59
;         temp[i] = x * x * 0.5f + x * 1.2f + 3.4f * sinf(x * 0.1f);
        v_mul_f32_e32 v2, 0x4059999a, v2                           // 000000002F78: 0A0404FF 4059999A
; /opt/rocm-7.1.0/lib/llvm/lib/clang/20/include/__clang_hip_math.h:707
; float sinf(float __x) { return __FAST_OR_SLOW(__sinf, __ocml_sin_f32)(__x); }
        v_cmp_class_f32_e64 vcc, v11, s26                          // 000000002F80: D010006A 0000350B
        v_cndmask_b32_e32 v2, v10, v2, vcc                         // 000000002F88: 0004050A
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:59
;         temp[i] = x * x * 0.5f + x * 1.2f + 3.4f * sinf(x * 0.1f);
        v_mov_b32_e32 v11, s14                                     // 000000002F8C: 7E16020E
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:57
;     for (int i = 0; i < 72; i++) {
        s_add_i32 s14, s14, 8                                      // 000000002F90: 810E880E
        s_add_i32 s27, s27, 1                                      // 000000002F94: 811B811B
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:59
;         temp[i] = x * x * 0.5f + x * 1.2f + 3.4f * sinf(x * 0.1f);
        v_add_f32_e32 v2, v15, v2                                  // 000000002F98: 0204050F
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:57
;     for (int i = 0; i < 72; i++) {
        s_cmpk_lg_i32 s27, 0x48                                    // 000000002F9C: B19B0048
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:59
;         temp[i] = x * x * 0.5f + x * 1.2f + 3.4f * sinf(x * 0.1f);
        buffer_store_dword v2, v11, s[0:3], 0 offen offset:4       // 000000002FA0: E0701004 8000020B
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:57
;     for (int i = 0; i < 72; i++) {
        s_cbranch_scc0 L8                                          // 000000002FA8: BF840163

0000000000002fac <L5>:
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:58
;         float x = in[idx] + i * 0.01f;
        v_cvt_f32_u32_e32 v2, s27                                  // 000000002FAC: 7E040C1B
        v_mov_b32_e32 v13, v4                                      // 000000002FB0: 7E1A0304
        v_fmac_f32_e32 v13, 0x3c23d70a, v2                         // 000000002FB4: 761A04FF 3C23D70A
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:59
;         temp[i] = x * x * 0.5f + x * 1.2f + 3.4f * sinf(x * 0.1f);
        v_mul_f32_e32 v11, 0x3dcccccd, v13                         // 000000002FBC: 0A161AFF 3DCCCCCD
; /opt/rocm-7.1.0/lib/llvm/lib/clang/20/include/__clang_hip_math.h:707
; float sinf(float __x) { return __FAST_OR_SLOW(__sinf, __ocml_sin_f32)(__x); }
        v_and_b32_e32 v12, 0x7fffffff, v11                         // 000000002FC4: 261816FF 7FFFFFFF
        v_cmp_nlt_f32_e64 s[4:5], |v11|, s15                       // 000000002FCC: D04E0104 00001F0B
        s_and_saveexec_b64 s[6:7], s[4:5]                          // 000000002FD4: BE862004
        s_xor_b64 s[10:11], exec, s[6:7]                           // 000000002FD8: 888A067E
        s_cbranch_execz L6                                         // 000000002FDC: BF880083
        v_lshrrev_b32_e32 v2, 23, v12                              // 000000002FE0: 20041897
        v_add_u32_e32 v2, 0xffffff88, v2                           // 000000002FE4: 680404FF FFFFFF88
        v_cmp_lt_u32_e32 vcc, 63, v2                               // 000000002FEC: 7D9204BF
        v_cndmask_b32_e32 v14, 0, v8, vcc                          // 000000002FF0: 001C1080
        v_add_u32_e32 v2, v14, v2                                  // 000000002FF4: 6804050E
        v_cmp_lt_u32_e64 s[4:5], 31, v2                            // 000000002FF8: D0C90004 0002049F
        v_cndmask_b32_e64 v14, 0, v9, s[4:5]                       // 000000003000: D100000E 00121280
        v_add_u32_e32 v2, v14, v2                                  // 000000003008: 6804050E
        v_cmp_lt_u32_e64 s[6:7], 31, v2                            // 00000000300C: D0C90006 0002049F
        v_cndmask_b32_e64 v14, 0, v9, s[6:7]                       // 000000003014: D100000E 001A1280
        v_add_u32_e32 v28, v14, v2                                 // 00000000301C: 6838050E
        v_and_b32_e32 v2, 0x7fffff, v12                            // 000000003020: 260418FF 007FFFFF
        v_or_b32_e32 v26, 0x800000, v2                             // 000000003028: 283404FF 00800000
        v_mad_u64_u32 v[14:15], s[8:9], v26, s16, 0                // 000000003030: D1E8080E 0200211A
        v_mov_b32_e32 v2, v15                                      // 000000003038: 7E04030F
        v_mad_u64_u32 v[16:17], s[8:9], v26, s17, v[2:3]           // 00000000303C: D1E80810 0408231A
        v_mov_b32_e32 v2, v17                                      // 000000003044: 7E040311
        v_mad_u64_u32 v[18:19], s[8:9], v26, s18, v[2:3]           // 000000003048: D1E80812 0408251A
        v_mov_b32_e32 v2, v19                                      // 000000003050: 7E040313
        v_mad_u64_u32 v[20:21], s[8:9], v26, s19, v[2:3]           // 000000003054: D1E80814 0408271A
        v_mov_b32_e32 v2, v21                                      // 00000000305C: 7E040315
        v_mad_u64_u32 v[22:23], s[8:9], v26, s20, v[2:3]           // 000000003060: D1E80816 0408291A
        v_mov_b32_e32 v2, v23                                      // 000000003068: 7E040317
        v_mad_u64_u32 v[24:25], s[8:9], v26, s21, v[2:3]           // 00000000306C: D1E80818 04082B1A
        v_mov_b32_e32 v2, v25                                      // 000000003074: 7E040319
        v_mad_u64_u32 v[26:27], s[8:9], v26, s22, v[2:3]           // 000000003078: D1E8081A 04082D1A
        v_cndmask_b32_e32 v15, v24, v20, vcc                       // 000000003080: 001E2918
        v_cndmask_b32_e32 v2, v26, v22, vcc                        // 000000003084: 00042D1A
        v_cndmask_b32_e32 v19, v27, v24, vcc                       // 000000003088: 0026311B
        v_cndmask_b32_e64 v17, v2, v15, s[4:5]                     // 00000000308C: D1000011 00121F02
        v_cndmask_b32_e64 v2, v19, v2, s[4:5]                      // 000000003094: D1000002 00120513
        v_cndmask_b32_e32 v19, v22, v18, vcc                       // 00000000309C: 00262516
        v_cndmask_b32_e64 v15, v15, v19, s[4:5]                    // 0000000030A0: D100000F 0012270F
        v_cndmask_b32_e32 v16, v20, v16, vcc                       // 0000000030A8: 00202114
        v_cndmask_b32_e64 v2, v2, v17, s[6:7]                      // 0000000030AC: D1000002 001A2302
        v_cndmask_b32_e64 v17, v17, v15, s[6:7]                    // 0000000030B4: D1000011 001A1F11
        v_sub_u32_e32 v21, 32, v28                                 // 0000000030BC: 6A2A38A0
        v_cndmask_b32_e64 v19, v19, v16, s[4:5]                    // 0000000030C0: D1000013 00122113
        v_alignbit_b32 v22, v2, v17, v21                           // 0000000030C8: D1CE0016 04562302
        v_cmp_eq_u32_e64 s[8:9], 0, v28                            // 0000000030D0: D0CA0008 00023880
        v_cndmask_b32_e64 v15, v15, v19, s[6:7]                    // 0000000030D8: D100000F 001A270F
        v_cndmask_b32_e32 v14, v18, v14, vcc                       // 0000000030E0: 001C1D12
        v_cndmask_b32_e64 v2, v22, v2, s[8:9]                      // 0000000030E4: D1000002 00220516
        v_alignbit_b32 v20, v17, v15, v21                          // 0000000030EC: D1CE0014 04561F11
        v_cndmask_b32_e64 v14, v16, v14, s[4:5]                    // 0000000030F4: D100000E 00121D10
        v_cndmask_b32_e64 v17, v20, v17, s[8:9]                    // 0000000030FC: D1000011 00222314
        v_bfe_u32 v23, v2, 29, 1                                   // 000000003104: D1C80017 02053B02
        v_cndmask_b32_e64 v14, v19, v14, s[6:7]                    // 00000000310C: D100000E 001A1D13
        v_alignbit_b32 v20, v2, v17, 30                            // 000000003114: D1CE0014 027A2302
        v_sub_u32_e32 v24, 0, v23                                  // 00000000311C: 6A302E80
        v_alignbit_b32 v16, v15, v14, v21                          // 000000003120: D1CE0010 04561D0F
        v_xor_b32_e32 v25, v20, v24                                // 000000003128: 2A323114
        v_cndmask_b32_e64 v15, v16, v15, s[8:9]                    // 00000000312C: D100000F 00221F10
        v_alignbit_b32 v16, v17, v15, 30                           // 000000003134: D1CE0010 027A1F11
        v_ffbh_u32_e32 v17, v25                                    // 00000000313C: 7E225B19
        v_add_u32_e32 v17, 1, v17                                  // 000000003140: 68222281
        v_cmp_ne_u32_e32 vcc, v20, v24                             // 000000003144: 7D9A3114
        v_cndmask_b32_e32 v17, 33, v17, vcc                        // 000000003148: 002222A1
        v_alignbit_b32 v14, v15, v14, 30                           // 00000000314C: D1CE000E 027A1D0F
        v_xor_b32_e32 v16, v16, v24                                // 000000003154: 2A203110
        v_sub_u32_e32 v18, 32, v17                                 // 000000003158: 6A2422A0
        v_xor_b32_e32 v14, v14, v24                                // 00000000315C: 2A1C310E
        v_alignbit_b32 v19, v25, v16, v18                          // 000000003160: D1CE0013 044A2119
        v_alignbit_b32 v14, v16, v14, v18                          // 000000003168: D1CE000E 044A1D10
        v_alignbit_b32 v15, v19, v14, 9                            // 000000003170: D1CE000F 02261D13
        v_ffbh_u32_e32 v16, v15                                    // 000000003178: 7E205B0F
        v_min_u32_e32 v16, 32, v16                                 // 00000000317C: 1C2020A0
        v_lshrrev_b32_e32 v22, 29, v2                              // 000000003180: 202C049D
        v_not_b32_e32 v18, v16                                     // 000000003184: 7E245710
        v_alignbit_b32 v14, v15, v14, v18                          // 000000003188: D1CE000E 044A1D0F
        v_lshlrev_b32_e32 v15, 31, v22                             // 000000003190: 241E2C9F
        v_or_b32_e32 v18, 0x33800000, v15                          // 000000003194: 28241EFF 33800000
        v_add_lshl_u32 v16, v16, v17, 23                           // 00000000319C: D1FE0010 025E2310
        v_lshrrev_b32_e32 v14, 9, v14                              // 0000000031A4: 201C1C89
        v_sub_u32_e32 v16, v18, v16                                // 0000000031A8: 6A202112
        v_or_b32_e32 v14, v16, v14                                 // 0000000031AC: 281C1D10
        v_alignbit_b32 v16, v17, v19, 9                            // 0000000031B0: D1CE0010 02262711
        v_or_b32_e32 v15, v16, v15                                 // 0000000031B8: 281E1F10
        v_xor_b32_e32 v15, 1.0, v15                                // 0000000031BC: 2A1E1EF2
        v_mul_f32_e32 v16, 0x3fc90fda, v15                         // 0000000031C0: 0A201EFF 3FC90FDA
        v_fma_f32 v17, v15, s23, -v16                              // 0000000031C8: D1CB0011 84402F0F
        v_fmac_f32_e32 v17, 0x33a22168, v15                        // 0000000031D0: 76221EFF 33A22168
        v_fmac_f32_e32 v17, 0x3fc90fda, v14                        // 0000000031D8: 76221CFF 3FC90FDA
        v_lshrrev_b32_e32 v2, 30, v2                               // 0000000031E0: 2004049E
        v_add_f32_e32 v14, v16, v17                                // 0000000031E4: 021C2310
        v_add_u32_e32 v2, v23, v2                                  // 0000000031E8: 68040517

00000000000031ec <L6>:
        s_andn2_saveexec_b64 s[4:5], s[10:11]                      // 0000000031EC: BE84230A
; /opt/rocm-7.1.0/lib/llvm/lib/clang/20/include/__clang_hip_math.h:707
; float sinf(float __x) { return __FAST_OR_SLOW(__sinf, __ocml_sin_f32)(__x); }
        v_mul_f32_e64 v2, |v11|, s24                               // 0000000031F0: D1050102 0000310B
        v_rndne_f32_e32 v15, v2                                    // 0000000031F8: 7E1E3D02
        v_cvt_i32_f32_e32 v2, v15                                  // 0000000031FC: 7E04110F
        v_fma_f32 v14, v15, s25, |v11|                             // 000000003200: D1CB040E 042C330F
        v_fmac_f32_e32 v14, 0xb3a22168, v15                        // 000000003208: 761C1EFF B3A22168
        v_fmac_f32_e32 v14, 0xa7c234c4, v15                        // 000000003210: 761C1EFF A7C234C4
        s_or_b64 exec, exec, s[4:5]                                // 000000003218: 87FE047E
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:59
;         temp[i] = x * x * 0.5f + x * 1.2f + 3.4f * sinf(x * 0.1f);
        v_mul_f32_e32 v15, v13, v13                                // 00000000321C: 0A1E1B0D
        v_mul_f32_e32 v15, 0.5, v15                                // 000000003220: 0A1E1EF0
        v_fmac_f32_e32 v15, 0x3f99999a, v13                        // 000000003224: 761E1AFF 3F99999A
; /opt/rocm-7.1.0/lib/llvm/lib/clang/20/include/__clang_hip_math.h:707
; float sinf(float __x) { return __FAST_OR_SLOW(__sinf, __ocml_sin_f32)(__x); }
        v_mul_f32_e32 v13, v14, v14                                // 00000000322C: 0A1A1D0E
        v_mov_b32_e32 v16, 0x3c0881c4                              // 000000003230: 7E2002FF 3C0881C4
        v_fmac_f32_e32 v16, 0xb94c1982, v13                        // 000000003238: 76201AFF B94C1982
        v_fma_f32 v16, v13, v16, v5                                // 000000003240: D1CB0010 0416210D
        v_mul_f32_e32 v16, v13, v16                                // 000000003248: 0A20210D
        v_fmac_f32_e32 v14, v14, v16                               // 00000000324C: 761C210E
        v_mov_b32_e32 v16, 0xbab64f3b                              // 000000003250: 7E2002FF BAB64F3B
        v_fmac_f32_e32 v16, 0x37d75334, v13                        // 000000003258: 76201AFF 37D75334
        v_fma_f32 v16, v13, v16, v6                                // 000000003260: D1CB0010 041A210D
        v_fma_f32 v16, v13, v16, v7                                // 000000003268: D1CB0010 041E210D
        v_fma_f32 v13, v13, v16, 1.0                               // 000000003270: D1CB000D 03CA210D
        v_and_b32_e32 v16, 1, v2                                   // 000000003278: 26200481
        v_cmp_eq_u32_e32 vcc, 0, v16                               // 00000000327C: 7D942080
        v_lshlrev_b32_e32 v2, 30, v2                               // 000000003280: 2404049E
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:58
;         float x = in[idx] + i * 0.01f;
        s_add_i32 s27, s27, 1                                      // 000000003284: 811B811B
; /opt/rocm-7.1.0/lib/llvm/lib/clang/20/include/__clang_hip_math.h:707
; float sinf(float __x) { return __FAST_OR_SLOW(__sinf, __ocml_sin_f32)(__x); }
        v_cndmask_b32_e32 v13, v13, v14, vcc                       // 000000003288: 001A1D0D
        v_and_b32_e32 v2, 0x80000000, v2                           // 00000000328C: 260404FF 80000000
        v_xor_b32_e32 v12, v12, v11                                // 000000003294: 2A18170C
        v_cmp_class_f32_e64 vcc, v11, s26                          // 000000003298: D010006A 0000350B
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:58
;         float x = in[idx] + i * 0.01f;
        v_cvt_f32_u32_e32 v11, s27                                 // 0000000032A0: 7E160C1B
; /opt/rocm-7.1.0/lib/llvm/lib/clang/20/include/__clang_hip_math.h:707
; float sinf(float __x) { return __FAST_OR_SLOW(__sinf, __ocml_sin_f32)(__x); }
        v_xor_b32_e32 v2, v12, v2                                  // 0000000032A4: 2A04050C
        v_xor_b32_e32 v2, v2, v13                                  // 0000000032A8: 2A041B02
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:59
;         temp[i] = x * x * 0.5f + x * 1.2f + 3.4f * sinf(x * 0.1f);
        v_mul_f32_e32 v2, 0x4059999a, v2                           // 0000000032AC: 0A0404FF 4059999A
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:58
;         float x = in[idx] + i * 0.01f;
        v_mov_b32_e32 v13, v4                                      // 0000000032B4: 7E1A0304
; /opt/rocm-7.1.0/lib/llvm/lib/clang/20/include/__clang_hip_math.h:707
; float sinf(float __x) { return __FAST_OR_SLOW(__sinf, __ocml_sin_f32)(__x); }
        v_cndmask_b32_e32 v2, v10, v2, vcc                         // 0000000032B8: 0004050A
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:58
;         float x = in[idx] + i * 0.01f;
        v_fmac_f32_e32 v13, 0x3c23d70a, v11                        // 0000000032BC: 761A16FF 3C23D70A
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:59
;         temp[i] = x * x * 0.5f + x * 1.2f + 3.4f * sinf(x * 0.1f);
        v_add_f32_e32 v2, v15, v2                                  // 0000000032C4: 0204050F
        v_mov_b32_e32 v12, s14                                     // 0000000032C8: 7E18020E
        v_mul_f32_e32 v11, 0x3dcccccd, v13                         // 0000000032CC: 0A161AFF 3DCCCCCD
        buffer_store_dword v2, v12, s[0:3], 0 offen                // 0000000032D4: E0701000 8000020C
; /opt/rocm-7.1.0/lib/llvm/lib/clang/20/include/__clang_hip_math.h:707
; float sinf(float __x) { return __FAST_OR_SLOW(__sinf, __ocml_sin_f32)(__x); }
        v_and_b32_e32 v12, 0x7fffffff, v11                         // 0000000032DC: 261816FF 7FFFFFFF
        v_cmp_nlt_f32_e64 s[4:5], |v11|, s15                       // 0000000032E4: D04E0104 00001F0B
        s_and_saveexec_b64 s[6:7], s[4:5]                          // 0000000032EC: BE862004
        s_xor_b64 s[10:11], exec, s[6:7]                           // 0000000032F0: 888A067E
        s_cbranch_execz L7                                         // 0000000032F4: BF880083
        v_lshrrev_b32_e32 v2, 23, v12                              // 0000000032F8: 20041897
        v_add_u32_e32 v2, 0xffffff88, v2                           // 0000000032FC: 680404FF FFFFFF88
        v_cmp_lt_u32_e32 vcc, 63, v2                               // 000000003304: 7D9204BF
        v_cndmask_b32_e32 v14, 0, v8, vcc                          // 000000003308: 001C1080
        v_add_u32_e32 v2, v14, v2                                  // 00000000330C: 6804050E
        v_cmp_lt_u32_e64 s[4:5], 31, v2                            // 000000003310: D0C90004 0002049F
        v_cndmask_b32_e64 v14, 0, v9, s[4:5]                       // 000000003318: D100000E 00121280
        v_add_u32_e32 v2, v14, v2                                  // 000000003320: 6804050E
        v_cmp_lt_u32_e64 s[6:7], 31, v2                            // 000000003324: D0C90006 0002049F
        v_cndmask_b32_e64 v14, 0, v9, s[6:7]                       // 00000000332C: D100000E 001A1280
        v_add_u32_e32 v28, v14, v2                                 // 000000003334: 6838050E
        v_and_b32_e32 v2, 0x7fffff, v12                            // 000000003338: 260418FF 007FFFFF
        v_or_b32_e32 v26, 0x800000, v2                             // 000000003340: 283404FF 00800000
        v_mad_u64_u32 v[14:15], s[8:9], v26, s16, 0                // 000000003348: D1E8080E 0200211A
        v_mov_b32_e32 v2, v15                                      // 000000003350: 7E04030F
        v_mad_u64_u32 v[16:17], s[8:9], v26, s17, v[2:3]           // 000000003354: D1E80810 0408231A
        v_mov_b32_e32 v2, v17                                      // 00000000335C: 7E040311
        v_mad_u64_u32 v[18:19], s[8:9], v26, s18, v[2:3]           // 000000003360: D1E80812 0408251A
        v_mov_b32_e32 v2, v19                                      // 000000003368: 7E040313
        v_mad_u64_u32 v[20:21], s[8:9], v26, s19, v[2:3]           // 00000000336C: D1E80814 0408271A
        v_mov_b32_e32 v2, v21                                      // 000000003374: 7E040315
        v_mad_u64_u32 v[22:23], s[8:9], v26, s20, v[2:3]           // 000000003378: D1E80816 0408291A
        v_mov_b32_e32 v2, v23                                      // 000000003380: 7E040317
        v_mad_u64_u32 v[24:25], s[8:9], v26, s21, v[2:3]           // 000000003384: D1E80818 04082B1A
        v_mov_b32_e32 v2, v25                                      // 00000000338C: 7E040319
        v_mad_u64_u32 v[26:27], s[8:9], v26, s22, v[2:3]           // 000000003390: D1E8081A 04082D1A
        v_cndmask_b32_e32 v15, v24, v20, vcc                       // 000000003398: 001E2918
        v_cndmask_b32_e32 v2, v26, v22, vcc                        // 00000000339C: 00042D1A
        v_cndmask_b32_e32 v19, v27, v24, vcc                       // 0000000033A0: 0026311B
        v_cndmask_b32_e64 v17, v2, v15, s[4:5]                     // 0000000033A4: D1000011 00121F02
        v_cndmask_b32_e64 v2, v19, v2, s[4:5]                      // 0000000033AC: D1000002 00120513
        v_cndmask_b32_e32 v19, v22, v18, vcc                       // 0000000033B4: 00262516
        v_cndmask_b32_e64 v15, v15, v19, s[4:5]                    // 0000000033B8: D100000F 0012270F
        v_cndmask_b32_e32 v16, v20, v16, vcc                       // 0000000033C0: 00202114
        v_cndmask_b32_e64 v2, v2, v17, s[6:7]                      // 0000000033C4: D1000002 001A2302
        v_cndmask_b32_e64 v17, v17, v15, s[6:7]                    // 0000000033CC: D1000011 001A1F11
        v_sub_u32_e32 v21, 32, v28                                 // 0000000033D4: 6A2A38A0
        v_cndmask_b32_e64 v19, v19, v16, s[4:5]                    // 0000000033D8: D1000013 00122113
        v_alignbit_b32 v22, v2, v17, v21                           // 0000000033E0: D1CE0016 04562302
        v_cmp_eq_u32_e64 s[8:9], 0, v28                            // 0000000033E8: D0CA0008 00023880
        v_cndmask_b32_e64 v15, v15, v19, s[6:7]                    // 0000000033F0: D100000F 001A270F
        v_cndmask_b32_e32 v14, v18, v14, vcc                       // 0000000033F8: 001C1D12
        v_cndmask_b32_e64 v2, v22, v2, s[8:9]                      // 0000000033FC: D1000002 00220516
        v_alignbit_b32 v20, v17, v15, v21                          // 000000003404: D1CE0014 04561F11
        v_cndmask_b32_e64 v14, v16, v14, s[4:5]                    // 00000000340C: D100000E 00121D10
        v_cndmask_b32_e64 v17, v20, v17, s[8:9]                    // 000000003414: D1000011 00222314
        v_bfe_u32 v23, v2, 29, 1                                   // 00000000341C: D1C80017 02053B02
        v_cndmask_b32_e64 v14, v19, v14, s[6:7]                    // 000000003424: D100000E 001A1D13
        v_alignbit_b32 v20, v2, v17, 30                            // 00000000342C: D1CE0014 027A2302
        v_sub_u32_e32 v24, 0, v23                                  // 000000003434: 6A302E80
        v_alignbit_b32 v16, v15, v14, v21                          // 000000003438: D1CE0010 04561D0F
        v_xor_b32_e32 v25, v20, v24                                // 000000003440: 2A323114
        v_cndmask_b32_e64 v15, v16, v15, s[8:9]                    // 000000003444: D100000F 00221F10
        v_alignbit_b32 v16, v17, v15, 30                           // 00000000344C: D1CE0010 027A1F11
        v_ffbh_u32_e32 v17, v25                                    // 000000003454: 7E225B19
        v_add_u32_e32 v17, 1, v17                                  // 000000003458: 68222281
        v_cmp_ne_u32_e32 vcc, v20, v24                             // 00000000345C: 7D9A3114
        v_cndmask_b32_e32 v17, 33, v17, vcc                        // 000000003460: 002222A1
        v_alignbit_b32 v14, v15, v14, 30                           // 000000003464: D1CE000E 027A1D0F
        v_xor_b32_e32 v16, v16, v24                                // 00000000346C: 2A203110
        v_sub_u32_e32 v18, 32, v17                                 // 000000003470: 6A2422A0
        v_xor_b32_e32 v14, v14, v24                                // 000000003474: 2A1C310E
        v_alignbit_b32 v19, v25, v16, v18                          // 000000003478: D1CE0013 044A2119
        v_alignbit_b32 v14, v16, v14, v18                          // 000000003480: D1CE000E 044A1D10
        v_alignbit_b32 v15, v19, v14, 9                            // 000000003488: D1CE000F 02261D13
        v_ffbh_u32_e32 v16, v15                                    // 000000003490: 7E205B0F
        v_min_u32_e32 v16, 32, v16                                 // 000000003494: 1C2020A0
        v_lshrrev_b32_e32 v22, 29, v2                              // 000000003498: 202C049D
        v_not_b32_e32 v18, v16                                     // 00000000349C: 7E245710
        v_alignbit_b32 v14, v15, v14, v18                          // 0000000034A0: D1CE000E 044A1D0F
        v_lshlrev_b32_e32 v15, 31, v22                             // 0000000034A8: 241E2C9F
        v_or_b32_e32 v18, 0x33800000, v15                          // 0000000034AC: 28241EFF 33800000
        v_add_lshl_u32 v16, v16, v17, 23                           // 0000000034B4: D1FE0010 025E2310
        v_lshrrev_b32_e32 v14, 9, v14                              // 0000000034BC: 201C1C89
        v_sub_u32_e32 v16, v18, v16                                // 0000000034C0: 6A202112
        v_or_b32_e32 v14, v16, v14                                 // 0000000034C4: 281C1D10
        v_alignbit_b32 v16, v17, v19, 9                            // 0000000034C8: D1CE0010 02262711
        v_or_b32_e32 v15, v16, v15                                 // 0000000034D0: 281E1F10
        v_xor_b32_e32 v15, 1.0, v15                                // 0000000034D4: 2A1E1EF2
        v_mul_f32_e32 v16, 0x3fc90fda, v15                         // 0000000034D8: 0A201EFF 3FC90FDA
        v_fma_f32 v17, v15, s23, -v16                              // 0000000034E0: D1CB0011 84402F0F
        v_fmac_f32_e32 v17, 0x33a22168, v15                        // 0000000034E8: 76221EFF 33A22168
        v_fmac_f32_e32 v17, 0x3fc90fda, v14                        // 0000000034F0: 76221CFF 3FC90FDA
        v_lshrrev_b32_e32 v2, 30, v2                               // 0000000034F8: 2004049E
        v_add_f32_e32 v14, v16, v17                                // 0000000034FC: 021C2310
        v_add_u32_e32 v2, v23, v2                                  // 000000003500: 68040517

0000000000003504 <L7>:
        s_andn2_saveexec_b64 s[4:5], s[10:11]                      // 000000003504: BE84230A
        s_cbranch_execz L4                                         // 000000003508: BF88FE7A
; /opt/rocm-7.1.0/lib/llvm/lib/clang/20/include/__clang_hip_math.h:707
; float sinf(float __x) { return __FAST_OR_SLOW(__sinf, __ocml_sin_f32)(__x); }
        v_mul_f32_e64 v2, |v11|, s24                               // 00000000350C: D1050102 0000310B
        v_rndne_f32_e32 v15, v2                                    // 000000003514: 7E1E3D02
        v_cvt_i32_f32_e32 v2, v15                                  // 000000003518: 7E04110F
        v_fma_f32 v14, v15, s25, |v11|                             // 00000000351C: D1CB040E 042C330F
        v_fmac_f32_e32 v14, 0xb3a22168, v15                        // 000000003524: 761C1EFF B3A22168
        v_fmac_f32_e32 v14, 0xa7c234c4, v15                        // 00000000352C: 761C1EFF A7C234C4
        s_branch L4                                                // 000000003534: BF82FE6F

0000000000003538 <L8>:
        s_mov_b32 s4, 0                                            // 000000003538: BE840080
        s_movk_i32 s5, 0x8c                                        // 00000000353C: B005008C
        v_mov_b32_e32 v3, 0                                        // 000000003540: 7E060280
        v_mov_b32_e32 v2, 0                                        // 000000003544: 7E040280

0000000000003548 <L9>:
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:65
;         sum1 += temp[i] * temp[71 - i];
        v_mov_b32_e32 v4, s4                                       // 000000003548: 7E080204
        v_mov_b32_e32 v5, s5                                       // 00000000354C: 7E0A0205
        buffer_load_dword v6, v4, s[0:3], 0 offen                  // 000000003550: E0501000 80000604
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:66
;         sum2 += temp[i + 36] * temp[35 - i];
        buffer_load_dword v7, v4, s[0:3], 0 offen offset:144       // 000000003558: E0501090 80000704
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:65
;         sum1 += temp[i] * temp[71 - i];
        buffer_load_dword v8, v5, s[0:3], 0 offen offset:144       // 000000003560: E0501090 80000805
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:66
;         sum2 += temp[i + 36] * temp[35 - i];
        buffer_load_dword v9, v5, s[0:3], 0 offen                  // 000000003568: E0501000 80000905
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:64
;     for (int i = 0; i < 36; i++) {
        s_add_i32 s5, s5, -4                                       // 000000003570: 8105C405
        s_add_i32 s4, s4, 4                                        // 000000003574: 81048404
        s_cmp_eq_u32 s5, -4                                        // 000000003578: BF06C405
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:65
;         sum1 += temp[i] * temp[71 - i];
        s_waitcnt vmcnt(1)                                         // 00000000357C: BF8C0F71
        v_fmac_f32_e32 v2, v6, v8                                  // 000000003580: 76041106
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:66
;         sum2 += temp[i + 36] * temp[35 - i];
        s_waitcnt vmcnt(0)                                         // 000000003584: BF8C0F70
        v_fmac_f32_e32 v3, v7, v9                                  // 000000003588: 76061307
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:64
;     for (int i = 0; i < 36; i++) {
        s_cbranch_scc0 L9                                          // 00000000358C: BF84FFEE
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:69
;     out[idx] = sum1 + sum2 * 0.7f;
        v_fmac_f32_e32 v2, 0x3f333333, v3                          // 000000003590: 760406FF 3F333333
        v_mov_b32_e32 v3, s13                                      // 000000003598: 7E06020D
        v_add_co_u32_e32 v0, vcc, s12, v0                          // 00000000359C: 3200000C
        v_addc_co_u32_e32 v1, vcc, v3, v1, vcc                     // 0000000035A0: 38020303
        global_store_dword v[0:1], v2, off                         // 0000000035A4: DC708000 007F0200

00000000000035ac <L10>:
; /home/ge64cax2/GPUscout/examples/amd/reg2-spill-vec.cpp:70
; }
        s_endpgm                                                   // 0000000035AC: BF810000
        s_nop 0                                                    // 0000000035B0: BF800000
        s_nop 0                                                    // 0000000035B4: BF800000
        s_nop 0                                                    // 0000000035B8: BF800000
        s_nop 0                                                    // 0000000035BC: BF800000
        s_nop 0                                                    // 0000000035C0: BF800000
        s_nop 0                                                    // 0000000035C4: BF800000
        s_nop 0                                                    // 0000000035C8: BF800000
        s_nop 0                                                    // 0000000035CC: BF800000
        s_nop 0                                                    // 0000000035D0: BF800000
        s_nop 0                                                    // 0000000035D4: BF800000
        s_nop 0                                                    // 0000000035D8: BF800000
        s_nop 0                                                    // 0000000035DC: BF800000
        s_nop 0                                                    // 0000000035E0: BF800000
        s_nop 0                                                    // 0000000035E4: BF800000
        s_nop 0                                                    // 0000000035E8: BF800000
        s_nop 0                                                    // 0000000035EC: BF800000
        s_nop 0                                                    // 0000000035F0: BF800000
        s_nop 0                                                    // 0000000035F4: BF800000
        s_nop 0                                                    // 0000000035F8: BF800000
        s_nop 0                                                    // 0000000035FC: BF800000
        s_nop 0                                                    // 000000003600: BF800000
        s_nop 0                                                    // 000000003604: BF800000
        s_nop 0                                                    // 000000003608: BF800000
        s_nop 0                                                    // 00000000360C: BF800000
        s_nop 0                                                    // 000000003610: BF800000
        s_nop 0                                                    // 000000003614: BF800000
        s_nop 0                                                    // 000000003618: BF800000
        s_nop 0                                                    // 00000000361C: BF800000
        s_nop 0                                                    // 000000003620: BF800000
        s_nop 0                                                    // 000000003624: BF800000
        s_nop 0                                                    // 000000003628: BF800000
        s_nop 0                                                    // 00000000362C: BF800000
        s_nop 0                                                    // 000000003630: BF800000
        s_nop 0                                                    // 000000003634: BF800000
        s_nop 0                                                    // 000000003638: BF800000
        s_nop 0                                                    // 00000000363C: BF800000
        s_nop 0                                                    // 000000003640: BF800000
        s_nop 0                                                    // 000000003644: BF800000
        s_nop 0                                                    // 000000003648: BF800000
        s_nop 0                                                    // 00000000364C: BF800000
        s_nop 0                                                    // 000000003650: BF800000
        s_nop 0                                                    // 000000003654: BF800000
        s_nop 0                                                    // 000000003658: BF800000
        s_nop 0                                                    // 00000000365C: BF800000
        s_nop 0                                                    // 000000003660: BF800000
        s_nop 0                                                    // 000000003664: BF800000
        s_nop 0                                                    // 000000003668: BF800000
        s_nop 0                                                    // 00000000366C: BF800000
        s_nop 0                                                    // 000000003670: BF800000
        s_nop 0                                                    // 000000003674: BF800000
        s_nop 0                                                    // 000000003678: BF800000
        s_nop 0                                                    // 00000000367C: BF800000
        s_nop 0                                                    // 000000003680: BF800000
        s_nop 0                                                    // 000000003684: BF800000
        s_nop 0                                                    // 000000003688: BF800000
        s_nop 0                                                    // 00000000368C: BF800000
        s_nop 0                                                    // 000000003690: BF800000
        s_nop 0                                                    // 000000003694: BF800000
        s_nop 0                                                    // 000000003698: BF800000
        s_nop 0                                                    // 00000000369C: BF800000
        s_nop 0                                                    // 0000000036A0: BF800000
        s_nop 0                                                    // 0000000036A4: BF800000
        s_nop 0                                                    // 0000000036A8: BF800000
        s_nop 0                                                    // 0000000036AC: BF800000
        s_nop 0                                                    // 0000000036B0: BF800000
        s_nop 0                                                    // 0000000036B4: BF800000
        s_nop 0                                                    // 0000000036B8: BF800000
        s_nop 0                                                    // 0000000036BC: BF800000
        s_nop 0                                                    // 0000000036C0: BF800000
        s_nop 0                                                    // 0000000036C4: BF800000
        s_nop 0                                                    // 0000000036C8: BF800000
        s_nop 0                                                    // 0000000036CC: BF800000
        s_nop 0                                                    // 0000000036D0: BF800000
        s_nop 0                                                    // 0000000036D4: BF800000
        s_nop 0                                                    // 0000000036D8: BF800000
        s_nop 0                                                    // 0000000036DC: BF800000
        s_nop 0                                                    // 0000000036E0: BF800000
        s_nop 0                                                    // 0000000036E4: BF800000
        s_nop 0                                                    // 0000000036E8: BF800000
        s_nop 0                                                    // 0000000036EC: BF800000
        s_nop 0                                                    // 0000000036F0: BF800000
        s_nop 0                                                    // 0000000036F4: BF800000
        s_nop 0                                                    // 0000000036F8: BF800000
        s_nop 0                                                    // 0000000036FC: BF800000
        s_nop 0                                                    // 000000003700: BF800000
        s_nop 0                                                    // 000000003704: BF800000
        s_nop 0                                                    // 000000003708: BF800000
        s_nop 0                                                    // 00000000370C: BF800000
        s_nop 0                                                    // 000000003710: BF800000
        s_nop 0                                                    // 000000003714: BF800000
        s_nop 0                                                    // 000000003718: BF800000
        s_nop 0                                                    // 00000000371C: BF800000
        s_nop 0                                                    // 000000003720: BF800000
        s_nop 0                                                    // 000000003724: BF800000
        s_nop 0                                                    // 000000003728: BF800000
        s_nop 0                                                    // 00000000372C: BF800000
        s_nop 0                                                    // 000000003730: BF800000
        s_nop 0                                                    // 000000003734: BF800000
        s_nop 0                                                    // 000000003738: BF800000
        s_nop 0                                                    // 00000000373C: BF800000
        s_nop 0                                                    // 000000003740: BF800000
        s_nop 0                                                    // 000000003744: BF800000
        s_nop 0                                                    // 000000003748: BF800000
        s_nop 0                                                    // 00000000374C: BF800000
        s_nop 0                                                    // 000000003750: BF800000
        s_nop 0                                                    // 000000003754: BF800000
        s_nop 0                                                    // 000000003758: BF800000
        s_nop 0                                                    // 00000000375C: BF800000
        s_nop 0                                                    // 000000003760: BF800000
        s_nop 0                                                    // 000000003764: BF800000
        s_nop 0                                                    // 000000003768: BF800000
        s_nop 0                                                    // 00000000376C: BF800000
        s_nop 0                                                    // 000000003770: BF800000
        s_nop 0                                                    // 000000003774: BF800000
        s_nop 0                                                    // 000000003778: BF800000
        s_nop 0                                                    // 00000000377C: BF800000
        s_nop 0                                                    // 000000003780: BF800000
        s_nop 0                                                    // 000000003784: BF800000
        s_nop 0                                                    // 000000003788: BF800000
        s_nop 0                                                    // 00000000378C: BF800000
        s_nop 0                                                    // 000000003790: BF800000
        s_nop 0                                                    // 000000003794: BF800000
        s_nop 0                                                    // 000000003798: BF800000
        s_nop 0                                                    // 00000000379C: BF800000
        s_nop 0                                                    // 0000000037A0: BF800000
        s_nop 0                                                    // 0000000037A4: BF800000
        s_nop 0                                                    // 0000000037A8: BF800000
        s_nop 0                                                    // 0000000037AC: BF800000
        s_nop 0                                                    // 0000000037B0: BF800000
        s_nop 0                                                    // 0000000037B4: BF800000
        s_nop 0                                                    // 0000000037B8: BF800000
        s_nop 0                                                    // 0000000037BC: BF800000
        s_nop 0                                                    // 0000000037C0: BF800000
        s_nop 0                                                    // 0000000037C4: BF800000
        s_nop 0                                                    // 0000000037C8: BF800000
        s_nop 0                                                    // 0000000037CC: BF800000
        s_nop 0                                                    // 0000000037D0: BF800000
        s_nop 0                                                    // 0000000037D4: BF800000
        s_nop 0                                                    // 0000000037D8: BF800000
        s_nop 0                                                    // 0000000037DC: BF800000
        s_nop 0                                                    // 0000000037E0: BF800000
        s_nop 0                                                    // 0000000037E4: BF800000
        s_nop 0                                                    // 0000000037E8: BF800000
        s_nop 0                                                    // 0000000037EC: BF800000
        s_nop 0                                                    // 0000000037F0: BF800000
        s_nop 0                                                    // 0000000037F4: BF800000
        s_nop 0                                                    // 0000000037F8: BF800000
        s_nop 0                                                    // 0000000037FC: BF800000
        s_nop 0                                                    // 000000003800: BF800000
        s_nop 0                                                    // 000000003804: BF800000
        s_nop 0                                                    // 000000003808: BF800000
        s_nop 0                                                    // 00000000380C: BF800000
        s_nop 0                                                    // 000000003810: BF800000
        s_nop 0                                                    // 000000003814: BF800000
        s_nop 0                                                    // 000000003818: BF800000
        s_nop 0                                                    // 00000000381C: BF800000
        s_nop 0                                                    // 000000003820: BF800000
        s_nop 0                                                    // 000000003824: BF800000
        s_nop 0                                                    // 000000003828: BF800000
        s_nop 0                                                    // 00000000382C: BF800000
        s_nop 0                                                    // 000000003830: BF800000
        s_nop 0                                                    // 000000003834: BF800000
        s_nop 0                                                    // 000000003838: BF800000
        s_nop 0                                                    // 00000000383C: BF800000
        s_nop 0                                                    // 000000003840: BF800000
        s_nop 0                                                    // 000000003844: BF800000
        s_nop 0                                                    // 000000003848: BF800000
        s_nop 0                                                    // 00000000384C: BF800000
        s_nop 0                                                    // 000000003850: BF800000
        s_nop 0                                                    // 000000003854: BF800000
        s_nop 0                                                    // 000000003858: BF800000
        s_nop 0                                                    // 00000000385C: BF800000
        s_nop 0                                                    // 000000003860: BF800000
        s_nop 0                                                    // 000000003864: BF800000
        s_nop 0                                                    // 000000003868: BF800000
        s_nop 0                                                    // 00000000386C: BF800000
        s_nop 0                                                    // 000000003870: BF800000
        s_nop 0                                                    // 000000003874: BF800000
        s_nop 0                                                    // 000000003878: BF800000
        s_nop 0                                                    // 00000000387C: BF800000
        s_nop 0                                                    // 000000003880: BF800000
        s_nop 0                                                    // 000000003884: BF800000
        s_nop 0                                                    // 000000003888: BF800000
        s_nop 0                                                    // 00000000388C: BF800000
        s_nop 0                                                    // 000000003890: BF800000
        s_nop 0                                                    // 000000003894: BF800000
        s_nop 0                                                    // 000000003898: BF800000
        s_nop 0                                                    // 00000000389C: BF800000
        s_nop 0                                                    // 0000000038A0: BF800000
        s_nop 0                                                    // 0000000038A4: BF800000
        s_nop 0                                                    // 0000000038A8: BF800000
        s_nop 0                                                    // 0000000038AC: BF800000
        s_nop 0                                                    // 0000000038B0: BF800000
        s_nop 0                                                    // 0000000038B4: BF800000
        s_nop 0                                                    // 0000000038B8: BF800000
        s_nop 0                                                    // 0000000038BC: BF800000
        s_nop 0                                                    // 0000000038C0: BF800000
        s_nop 0                                                    // 0000000038C4: BF800000
        s_nop 0                                                    // 0000000038C8: BF800000
        s_nop 0                                                    // 0000000038CC: BF800000
        s_nop 0                                                    // 0000000038D0: BF800000
        s_nop 0                                                    // 0000000038D4: BF800000
        s_nop 0                                                    // 0000000038D8: BF800000
        s_nop 0                                                    // 0000000038DC: BF800000
        s_nop 0                                                    // 0000000038E0: BF800000
        s_nop 0                                                    // 0000000038E4: BF800000
        s_nop 0                                                    // 0000000038E8: BF800000
        s_nop 0                                                    // 0000000038EC: BF800000
        s_nop 0                                                    // 0000000038F0: BF800000
        s_nop 0                                                    // 0000000038F4: BF800000
        s_nop 0                                                    // 0000000038F8: BF800000
        s_nop 0                                                    // 0000000038FC: BF800000
        s_nop 0                                                    // 000000003900: BF800000
        s_nop 0                                                    // 000000003904: BF800000
        s_nop 0                                                    // 000000003908: BF800000
        s_nop 0                                                    // 00000000390C: BF800000
        s_nop 0                                                    // 000000003910: BF800000
        s_nop 0                                                    // 000000003914: BF800000
        s_nop 0                                                    // 000000003918: BF800000
        s_nop 0                                                    // 00000000391C: BF800000
        s_nop 0                                                    // 000000003920: BF800000
        s_nop 0                                                    // 000000003924: BF800000
        s_nop 0                                                    // 000000003928: BF800000
        s_nop 0                                                    // 00000000392C: BF800000
        s_nop 0                                                    // 000000003930: BF800000
        s_nop 0                                                    // 000000003934: BF800000
        s_nop 0                                                    // 000000003938: BF800000
        s_nop 0                                                    // 00000000393C: BF800000
        s_nop 0                                                    // 000000003940: BF800000
        s_nop 0                                                    // 000000003944: BF800000
        s_nop 0                                                    // 000000003948: BF800000
        s_nop 0                                                    // 00000000394C: BF800000
        s_nop 0                                                    // 000000003950: BF800000
        s_nop 0                                                    // 000000003954: BF800000
        s_nop 0                                                    // 000000003958: BF800000
        s_nop 0                                                    // 00000000395C: BF800000
        s_nop 0                                                    // 000000003960: BF800000
        s_nop 0                                                    // 000000003964: BF800000
        s_nop 0                                                    // 000000003968: BF800000
        s_nop 0                                                    // 00000000396C: BF800000
        s_nop 0                                                    // 000000003970: BF800000
        s_nop 0                                                    // 000000003974: BF800000
        s_nop 0                                                    // 000000003978: BF800000
        s_nop 0                                                    // 00000000397C: BF800000
        s_nop 0                                                    // 000000003980: BF800000
        s_nop 0                                                    // 000000003984: BF800000
        s_nop 0                                                    // 000000003988: BF800000
        s_nop 0                                                    // 00000000398C: BF800000
        s_nop 0                                                    // 000000003990: BF800000
        s_nop 0                                                    // 000000003994: BF800000
        s_nop 0                                                    // 000000003998: BF800000
        s_nop 0                                                    // 00000000399C: BF800000
        s_nop 0                                                    // 0000000039A0: BF800000
        s_nop 0                                                    // 0000000039A4: BF800000
        s_nop 0                                                    // 0000000039A8: BF800000
        s_nop 0                                                    // 0000000039AC: BF800000
        s_nop 0                                                    // 0000000039B0: BF800000
        s_nop 0                                                    // 0000000039B4: BF800000
        s_nop 0                                                    // 0000000039B8: BF800000
        s_nop 0                                                    // 0000000039BC: BF800000