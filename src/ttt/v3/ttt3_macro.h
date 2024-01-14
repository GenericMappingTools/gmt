/*
 * COPYRIGHT (c) 1993-2024 GEOWARE
 *
 * ttt3_macro.h
 *
 * PROGRAM:	ttt_macro.h
 * PURPOSE:	include file for ttt.c
 * AUTHOR:	Paul Wessel, GEOWARE
 * DATE:	June 16, 1993
 * UPDATED:	January 1, 2024
 */

/* Define macros to obtain average slowness to neigboring nodes */

#define THIRD		(1.0/3.0)
#define TWO_THIRD	(2.0/3.0)
#define FOUR_THIRD	(4.0/3.0)
#define FIVE_THIRD	(5.0/3.0)

#define S(X)		s[ij+p[X]]
#define GET_TT_INC(Y)	((TTT_LONG) ((Y) * ss - 0.5))

	/* First do sqrt(1) nodes */
			
#define SS_00 (s[ij] + S(0))
#define SS_01 (s[ij] + S(1))
#define SS_02 (s[ij] + S(2))
#define SS_03 (s[ij] + S(3))
			
	/* Then do sqrt(2) nodes */
			
#define SS_04 (s[ij] + S(4))
#define SS_05 (s[ij] + S(5))
#define SS_06 (s[ij] + S(6))
#define SS_07 (s[ij] + S(7))
			
	/* Then do sqrt(5) nodes */
			
#define SS_08 (s[ij] + S(0) + S(4) + S(8))
#define SS_09 (s[ij] + S(1) + S(5) + S(9))
#define SS_10 (s[ij] + S(0) + S(6) + S(10))
#define SS_11 (s[ij] + S(1) + S(7) + S(11))
#define SS_12 (s[ij] + S(3) + S(4) + S(12))
#define SS_13 (s[ij] + S(2) + S(5) + S(13))
#define SS_14 (s[ij] + S(2) + S(6) + S(14))
#define SS_15 (s[ij] + S(3) + S(7) + S(15))
			
	/* Do sqrt(10) nodes */
			
#define SS_16 (s[ij] + TWO_THIRD * (S(64) + S(4)) + FOUR_THIRD * (S(0) + S(8))  + S(16))
#define SS_17 (s[ij] + TWO_THIRD * (S(65) + S(5)) + FOUR_THIRD * (S(1) + S(9))  + S(17))
#define SS_18 (s[ij] + TWO_THIRD * (S(64) + S(6)) + FOUR_THIRD * (S(0) + S(10)) + S(18))
#define SS_19 (s[ij] + TWO_THIRD * (S(65) + S(7)) + FOUR_THIRD * (S(1) + S(11)) + S(19))
#define SS_20 (s[ij] + TWO_THIRD * (S(67) + S(4)) + FOUR_THIRD * (S(3) + S(12)) + S(20))
#define SS_21 (s[ij] + TWO_THIRD * (S(66) + S(5)) + FOUR_THIRD * (S(2) + S(13)) + S(21))
#define SS_22 (s[ij] + TWO_THIRD * (S(66) + S(6)) + FOUR_THIRD * (S(2) + S(14)) + S(22))
#define SS_23 (s[ij] + TWO_THIRD * (S(67) + S(7)) + FOUR_THIRD * (S(3) + S(15)) + S(23))
			
	/* Do sqrt(13) nodes */
			
#define SS_24 (s[ij] + 0.5 * (S(0) + S(68)) + 1.5 * (S(4) + S(8))  + S(24))
#define SS_25 (s[ij] + 0.5 * (S(1) + S(69)) + 1.5 * (S(5) + S(9))  + S(25))
#define SS_26 (s[ij] + 0.5 * (S(0) + S(70)) + 1.5 * (S(6) + S(10)) + S(26))
#define SS_27 (s[ij] + 0.5 * (S(1) + S(71)) + 1.5 * (S(7) + S(11)) + S(27))
#define SS_28 (s[ij] + 0.5 * (S(3) + S(68)) + 1.5 * (S(4) + S(12)) + S(28))
#define SS_29 (s[ij] + 0.5 * (S(2) + S(69)) + 1.5 * (S(5) + S(13)) + S(29))
#define SS_30 (s[ij] + 0.5 * (S(2) + S(70)) + 1.5 * (S(6) + S(14)) + S(30))
#define SS_31 (s[ij] + 0.5 * (S(3) + S(71)) + 1.5 * (S(7) + S(15)) + S(31))

	/* Do sqrt(17) nodes */
			
#define SS_32 (s[ij] + 0.5 * (S(72) + S(4)) + 1.5 * (S(0) + S(16)) + S(64) + S(8)  + S(32))
#define SS_33 (s[ij] + 0.5 * (S(73) + S(5)) + 1.5 * (S(1) + S(17)) + S(65) + S(9)  + S(33))
#define SS_34 (s[ij] + 0.5 * (S(72) + S(6)) + 1.5 * (S(0) + S(18)) + S(64) + S(10) + S(34))
#define SS_35 (s[ij] + 0.5 * (S(73) + S(7)) + 1.5 * (S(1) + S(19)) + S(65) + S(11) + S(35))
#define SS_36 (s[ij] + 0.5 * (S(75) + S(4)) + 1.5 * (S(3) + S(20)) + S(67) + S(12) + S(36))
#define SS_37 (s[ij] + 0.5 * (S(74) + S(5)) + 1.5 * (S(2) + S(21)) + S(66) + S(13) + S(37))
#define SS_38 (s[ij] + 0.5 * (S(74) + S(6)) + 1.5 * (S(2) + S(22)) + S(66) + S(14) + S(38))
#define SS_39 (s[ij] + 0.5 * (S(75) + S(7)) + 1.5 * (S(3) + S(23)) + S(67) + S(15) + S(39))

	/* Do sqrt(25) nodes */
			
#define SS_40 (s[ij] + THIRD * (S(0) + S(80)) + FIVE_THIRD * (S(4) + S(24)) + S(8)  + S(68) + S(40))
#define SS_41 (s[ij] + THIRD * (S(1) + S(81)) + FIVE_THIRD * (S(5) + S(25)) + S(9)  + S(69) + S(41))
#define SS_42 (s[ij] + THIRD * (S(0) + S(82)) + FIVE_THIRD * (S(6) + S(26)) + S(10) + S(70) + S(42))
#define SS_43 (s[ij] + THIRD * (S(1) + S(83)) + FIVE_THIRD * (S(7) + S(27)) + S(11) + S(71) + S(43))
#define SS_44 (s[ij] + THIRD * (S(3) + S(80)) + FIVE_THIRD * (S(4) + S(28)) + S(12) + S(68) + S(44))
#define SS_45 (s[ij] + THIRD * (S(2) + S(81)) + FIVE_THIRD * (S(5) + S(29)) + S(13) + S(69) + S(45))
#define SS_46 (s[ij] + THIRD * (S(2) + S(82)) + FIVE_THIRD * (S(6) + S(30)) + S(14) + S(70) + S(46))
#define SS_47 (s[ij] + THIRD * (S(3) + S(83)) + FIVE_THIRD * (S(7) + S(31)) + S(15) + S(71) + S(47))

	/* Do sqrt(26) nodes */
			
#define SS_48 (s[ij] + 1.6 * (S(0) + S(32)) + 0.4 * (S(4) + S(76)) + 0.8 * (S(8)  + S(72)) + 1.2 * (S(16) + S(64)) + S(48))
#define SS_49 (s[ij] + 1.6 * (S(1) + S(33)) + 0.4 * (S(5) + S(77)) + 0.8 * (S(9)  + S(73)) + 1.2 * (S(17) + S(65)) + S(49))
#define SS_50 (s[ij] + 1.6 * (S(0) + S(34)) + 0.4 * (S(6) + S(76)) + 0.8 * (S(10) + S(72)) + 1.2 * (S(18) + S(64)) + S(50))
#define SS_51 (s[ij] + 1.6 * (S(1) + S(35)) + 0.4 * (S(7) + S(77)) + 0.8 * (S(11) + S(73)) + 1.2 * (S(19) + S(65)) + S(51))
#define SS_52 (s[ij] + 1.6 * (S(3) + S(36)) + 0.4 * (S(4) + S(79)) + 0.8 * (S(12) + S(75)) + 1.2 * (S(20) + S(67)) + S(52))
#define SS_53 (s[ij] + 1.6 * (S(2) + S(37)) + 0.4 * (S(5) + S(78)) + 0.8 * (S(13) + S(74)) + 1.2 * (S(21) + S(66)) + S(53))
#define SS_54 (s[ij] + 1.6 * (S(2) + S(38)) + 0.4 * (S(6) + S(78)) + 0.8 * (S(14) + S(74)) + 1.2 * (S(22) + S(66)) + S(54))
#define SS_55 (s[ij] + 1.6 * (S(3) + S(39)) + 0.4 * (S(7) + S(79)) + 0.8 * (S(15) + S(75)) + 1.2 * (S(23) + S(67)) + S(55))

	/* Do sqrt(29) nodes */
			
#define SS_56 (s[ij] + 1.2 * (S(0) + S(84)) + 0.8 * (S(4) + S(32)) + 1.7 * (S(8) + S(16))  + 0.3 * (S(24) + S(64)) + S(56))
#define SS_57 (s[ij] + 1.2 * (S(1) + S(85)) + 0.8 * (S(5) + S(33)) + 1.7 * (S(9) + S(17))  + 0.3 * (S(25) + S(65)) + S(57))
#define SS_58 (s[ij] + 1.2 * (S(0) + S(86)) + 0.8 * (S(6) + S(34)) + 1.7 * (S(10) + S(18)) + 0.3 * (S(26) + S(64)) + S(58))
#define SS_59 (s[ij] + 1.2 * (S(1) + S(87)) + 0.8 * (S(7) + S(35)) + 1.7 * (S(11) + S(19)) + 0.3 * (S(27) + S(65)) + S(59))
#define SS_60 (s[ij] + 1.2 * (S(3) + S(88)) + 0.8 * (S(4) + S(36)) + 1.7 * (S(12) + S(20)) + 0.3 * (S(28) + S(67)) + S(60))
#define SS_61 (s[ij] + 1.2 * (S(2) + S(89)) + 0.8 * (S(5) + S(37)) + 1.7 * (S(13) + S(21)) + 0.3 * (S(29) + S(66)) + S(61))
#define SS_62 (s[ij] + 1.2 * (S(2) + S(90)) + 0.8 * (S(6) + S(38)) + 1.7 * (S(14) + S(22)) + 0.3 * (S(30) + S(66)) + S(62))
#define SS_63 (s[ij] + 1.2 * (S(3) + S(91)) + 0.8 * (S(7) + S(39)) + 1.7 * (S(15) + S(23)) + 0.3 * (S(31) + S(67)) + S(63))
