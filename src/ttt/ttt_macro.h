/*
 * COPYRIGHT (c) 1993-2024 GEOWARE
 *
 * ttt_macro.h
 *
 * PROGRAM:	ttt_macro.h
 * PURPOSE:	include file for ttt.c
 * AUTHOR:	Paul Wessel, GEOWARE
 * DATE:	June 16, 1993
 * UPDATED:	January 1, 2024
 */

/* Define 120 SS_xxx macros to obtain average slowness to neigboring nodes */

/* First some floating point constants needed */
#define ONE_3RD		(1.0/3.0)
#define TWO_3RD		(2.0/3.0)
#define FOUR_3RD	(4.0/3.0)
#define FIVE_3RD	(5.0/3.0)
#define FOUR_15TH	(4.0/15.0)
#define TWENTY6_15TH	(26.0/15.0)
#define SIX_7TH		(6.0/7.0)
#define THREE_7TH	(3.0/7.0)
#define EIGHT_7TH	(8.0/7.0)
#define FOUR_21ST	(4.0/21.0)
#define TEN_21ST	(10.0/21.0)
#define THIRTY2_21ST	(32.0/21.0)
#define THIRTY8_21ST	(38.0/21.0)
#define ONE_6TH		(1.0/6.0)
#define FIVE_6TH	(5.0/6.0)
#define SEVEN_6TH	(7.0/6.0)
#define ELEVEN_6TH	(11.0/6.0)
#define ONE_13TH	(1.0/13.0)
#define TWO_13TH	(2.0/13.0)
#define FOUR_13TH	(4.0/13.0)
#define SIX_13TH	(6.0/13.0)
#define EIGHT_13TH	(8.0/13.0)
#define TEN_13TH	(10.0/13.0)
#define TWELVE_13TH	(12.0/13.0)
#define FOURTEEN_13TH	(14.0/13.0)
#define SIXTEEN_13TH	(16.0/13.0)
#define EIGHTEEN_13TH	(18.0/13.0)
#define TWENTY_13TH	(20.0/13.0)
#define TWENTY2_13TH	(22.0/13.0)
#define TWENTY4_13TH	(24.0/13.0)

/* S(X) returns the slowness at the given node offset X via array offsets in p[X] */
#define S(X)		s[ij+p[X]]
/* Multiply distance and slowness and round to nearest whole integer */
#define GET_TT_INC(Y)	((TTT_LONG) ((Y) * ss - 0.5))

	/* First do sqrt(1) nodes */
			
#define SS_000 (s[ij] + S(0))
#define SS_001 (s[ij] + S(1))
#define SS_002 (s[ij] + S(2))
#define SS_003 (s[ij] + S(3))
			
	/* Then do sqrt(2) nodes */
			
#define SS_004 (s[ij] + S(4))
#define SS_005 (s[ij] + S(5))
#define SS_006 (s[ij] + S(6))
#define SS_007 (s[ij] + S(7))
			
	/* Then do sqrt(5) nodes */
			
#define SS_008 (s[ij] + S(0) + S(4) + S(8))
#define SS_009 (s[ij] + S(1) + S(5) + S(9))
#define SS_010 (s[ij] + S(0) + S(6) + S(10))
#define SS_011 (s[ij] + S(1) + S(7) + S(11))
#define SS_012 (s[ij] + S(2) + S(4) + S(12))
#define SS_013 (s[ij] + S(3) + S(5) + S(13))
#define SS_014 (s[ij] + S(3) + S(6) + S(14))
#define SS_015 (s[ij] + S(2) + S(7) + S(15))
			
	/* Do sqrt(10) nodes */
			
#define SS_016 (s[ij] + TWO_3RD * (S(120) + S(4)) + FOUR_3RD * (S(0) + S(8))  + S(16))
#define SS_017 (s[ij] + TWO_3RD * (S(121) + S(5)) + FOUR_3RD * (S(1) + S(9))  + S(17))
#define SS_018 (s[ij] + TWO_3RD * (S(120) + S(6)) + FOUR_3RD * (S(0) + S(10)) + S(18))
#define SS_019 (s[ij] + TWO_3RD * (S(121) + S(7)) + FOUR_3RD * (S(1) + S(11)) + S(19))
#define SS_020 (s[ij] + TWO_3RD * (S(122) + S(4)) + FOUR_3RD * (S(2) + S(12)) + S(20))
#define SS_021 (s[ij] + TWO_3RD * (S(123) + S(5)) + FOUR_3RD * (S(3) + S(13)) + S(21))
#define SS_022 (s[ij] + TWO_3RD * (S(123) + S(6)) + FOUR_3RD * (S(3) + S(14)) + S(22))
#define SS_023 (s[ij] + TWO_3RD * (S(122) + S(7)) + FOUR_3RD * (S(2) + S(15)) + S(23))
			
	/* Do sqrt(13) nodes */
			
#define SS_024 (s[ij] + 0.5 * (S(0) + S(124)) + 1.5 * (S(4) + S(8))  + S(24))
#define SS_025 (s[ij] + 0.5 * (S(1) + S(125)) + 1.5 * (S(5) + S(9))  + S(25))
#define SS_026 (s[ij] + 0.5 * (S(0) + S(126)) + 1.5 * (S(6) + S(10)) + S(26))
#define SS_027 (s[ij] + 0.5 * (S(1) + S(127)) + 1.5 * (S(7) + S(11)) + S(27))
#define SS_028 (s[ij] + 0.5 * (S(2) + S(124)) + 1.5 * (S(4) + S(12)) + S(28))
#define SS_029 (s[ij] + 0.5 * (S(3) + S(125)) + 1.5 * (S(5) + S(13)) + S(29))
#define SS_030 (s[ij] + 0.5 * (S(3) + S(126)) + 1.5 * (S(6) + S(14)) + S(30))
#define SS_031 (s[ij] + 0.5 * (S(2) + S(127)) + 1.5 * (S(7) + S(15)) + S(31))

	/* Do sqrt(17) nodes */
			
#define SS_032 (s[ij] + 0.5 * (S(128) + S(4)) + 1.5 * (S(0) + S(16)) + S(120) + S(8)  + S(32))
#define SS_033 (s[ij] + 0.5 * (S(129) + S(5)) + 1.5 * (S(1) + S(17)) + S(121) + S(9)  + S(33))
#define SS_034 (s[ij] + 0.5 * (S(128) + S(6)) + 1.5 * (S(0) + S(18)) + S(120) + S(10) + S(34))
#define SS_035 (s[ij] + 0.5 * (S(129) + S(7)) + 1.5 * (S(1) + S(19)) + S(121) + S(11) + S(35))
#define SS_036 (s[ij] + 0.5 * (S(130) + S(4)) + 1.5 * (S(2) + S(20)) + S(122) + S(12) + S(36))
#define SS_037 (s[ij] + 0.5 * (S(131) + S(5)) + 1.5 * (S(3) + S(21)) + S(123) + S(13) + S(37))
#define SS_038 (s[ij] + 0.5 * (S(131) + S(6)) + 1.5 * (S(3) + S(22)) + S(123) + S(14) + S(38))
#define SS_039 (s[ij] + 0.5 * (S(130) + S(7)) + 1.5 * (S(2) + S(23)) + S(122) + S(15) + S(39))

	/* Do sqrt(25) nodes */
			
#define SS_040 (s[ij] + ONE_3RD * (S(0) + S(136)) + FIVE_3RD * (S(4) + S(24)) + S(8)  + S(124) + S(40))
#define SS_041 (s[ij] + ONE_3RD * (S(1) + S(137)) + FIVE_3RD * (S(5) + S(25)) + S(9)  + S(125) + S(41))
#define SS_042 (s[ij] + ONE_3RD * (S(0) + S(138)) + FIVE_3RD * (S(6) + S(26)) + S(10) + S(126) + S(42))
#define SS_043 (s[ij] + ONE_3RD * (S(1) + S(139)) + FIVE_3RD * (S(7) + S(27)) + S(11) + S(127) + S(43))
#define SS_044 (s[ij] + ONE_3RD * (S(2) + S(136)) + FIVE_3RD * (S(4) + S(28)) + S(12) + S(124) + S(44))
#define SS_045 (s[ij] + ONE_3RD * (S(3) + S(137)) + FIVE_3RD * (S(5) + S(29)) + S(13) + S(125) + S(45))
#define SS_046 (s[ij] + ONE_3RD * (S(3) + S(138)) + FIVE_3RD * (S(6) + S(30)) + S(14) + S(126) + S(46))
#define SS_047 (s[ij] + ONE_3RD * (S(2) + S(139)) + FIVE_3RD * (S(7) + S(31)) + S(15) + S(127) + S(47))

	/* Do sqrt(26) nodes */
			
#define SS_048 (s[ij] + 1.6 * (S(0) + S(32)) + 0.4 * (S(4) + S(132)) + 0.8 * (S(8)  + S(128)) + 1.2 * (S(16) + S(120)) + S(48))
#define SS_049 (s[ij] + 1.6 * (S(1) + S(33)) + 0.4 * (S(5) + S(133)) + 0.8 * (S(9)  + S(129)) + 1.2 * (S(17) + S(121)) + S(49))
#define SS_050 (s[ij] + 1.6 * (S(0) + S(34)) + 0.4 * (S(6) + S(132)) + 0.8 * (S(10) + S(128)) + 1.2 * (S(18) + S(120)) + S(50))
#define SS_051 (s[ij] + 1.6 * (S(1) + S(35)) + 0.4 * (S(7) + S(133)) + 0.8 * (S(11) + S(129)) + 1.2 * (S(19) + S(121)) + S(51))
#define SS_052 (s[ij] + 1.6 * (S(2) + S(36)) + 0.4 * (S(4) + S(134)) + 0.8 * (S(12) + S(130)) + 1.2 * (S(20) + S(122)) + S(52))
#define SS_053 (s[ij] + 1.6 * (S(3) + S(37)) + 0.4 * (S(5) + S(135)) + 0.8 * (S(13) + S(131)) + 1.2 * (S(21) + S(123)) + S(53))
#define SS_054 (s[ij] + 1.6 * (S(3) + S(38)) + 0.4 * (S(6) + S(135)) + 0.8 * (S(14) + S(131)) + 1.2 * (S(22) + S(123)) + S(54))
#define SS_055 (s[ij] + 1.6 * (S(2) + S(39)) + 0.4 * (S(7) + S(134)) + 0.8 * (S(15) + S(130)) + 1.2 * (S(23) + S(122)) + S(55))

	/* Do sqrt(29) nodes */
			
#define SS_056 (s[ij] + 1.2 * (S(0) + S(140)) + 0.8 * (S(4) + S(32)) + 1.7 * (S(8)  + S(16)) + 0.3 * (S(24) + S(120)) + S(56))
#define SS_057 (s[ij] + 1.2 * (S(1) + S(141)) + 0.8 * (S(5) + S(33)) + 1.7 * (S(9)  + S(17)) + 0.3 * (S(25) + S(121)) + S(57))
#define SS_058 (s[ij] + 1.2 * (S(0) + S(142)) + 0.8 * (S(6) + S(34)) + 1.7 * (S(10) + S(18)) + 0.3 * (S(26) + S(120)) + S(58))
#define SS_059 (s[ij] + 1.2 * (S(1) + S(143)) + 0.8 * (S(7) + S(35)) + 1.7 * (S(11) + S(19)) + 0.3 * (S(27) + S(121)) + S(59))
#define SS_060 (s[ij] + 1.2 * (S(2) + S(144)) + 0.8 * (S(4) + S(36)) + 1.7 * (S(12) + S(20)) + 0.3 * (S(28) + S(122)) + S(60))
#define SS_061 (s[ij] + 1.2 * (S(3) + S(145)) + 0.8 * (S(5) + S(37)) + 1.7 * (S(13) + S(21)) + 0.3 * (S(29) + S(123)) + S(61))
#define SS_062 (s[ij] + 1.2 * (S(3) + S(146)) + 0.8 * (S(6) + S(38)) + 1.7 * (S(14) + S(22)) + 0.3 * (S(30) + S(123)) + S(62))
#define SS_063 (s[ij] + 1.2 * (S(2) + S(147)) + 0.8 * (S(7) + S(39)) + 1.7 * (S(15) + S(23)) + 0.3 * (S(31) + S(122)) + S(63))

	/* Do sqrt(34) nodes */

#define SS_064 (s[ij] + TWO_3RD * (S(0) + S(40)) + FOUR_3RD * (S(4) + S(140)) + TWENTY6_15TH * (S(8)  + S(24)) + FOUR_15TH * (S(16) + S(124)) + S(64))
#define SS_065 (s[ij] + TWO_3RD * (S(1) + S(41)) + FOUR_3RD * (S(5) + S(141)) + TWENTY6_15TH * (S(9)  + S(25)) + FOUR_15TH * (S(17) + S(125)) + S(65))
#define SS_066 (s[ij] + TWO_3RD * (S(0) + S(42)) + FOUR_3RD * (S(6) + S(142)) + TWENTY6_15TH * (S(10) + S(26)) + FOUR_15TH * (S(18) + S(126)) + S(66))
#define SS_067 (s[ij] + TWO_3RD * (S(1) + S(43)) + FOUR_3RD * (S(7) + S(143)) + TWENTY6_15TH * (S(11) + S(27)) + FOUR_15TH * (S(19) + S(127)) + S(67))
#define SS_068 (s[ij] + TWO_3RD * (S(2) + S(44)) + FOUR_3RD * (S(4) + S(144)) + TWENTY6_15TH * (S(12) + S(28)) + FOUR_15TH * (S(20) + S(124)) + S(68))
#define SS_069 (s[ij] + TWO_3RD * (S(3) + S(45)) + FOUR_3RD * (S(5) + S(145)) + TWENTY6_15TH * (S(13) + S(29)) + FOUR_15TH * (S(21) + S(125)) + S(69))
#define SS_070 (s[ij] + TWO_3RD * (S(3) + S(46)) + FOUR_3RD * (S(6) + S(146)) + TWENTY6_15TH * (S(14) + S(30)) + FOUR_15TH * (S(22) + S(126)) + S(70))
#define SS_071 (s[ij] + TWO_3RD * (S(2) + S(47)) + FOUR_3RD * (S(7) + S(147)) + TWENTY6_15TH * (S(15) + S(31)) + FOUR_15TH * (S(23) + S(127)) + S(71))

	/* Do sqrt(37) nodes */

#define SS_072 (s[ij] + FIVE_3RD * (S(0) + S(48)) + FOUR_3RD * (S(32) + S(120)) + S(16) + S(128) + TWO_3RD * (S(8)  + S(132)) + ONE_3RD * (S(4) + S(148)) + S(72))
#define SS_073 (s[ij] + FIVE_3RD * (S(1) + S(49)) + FOUR_3RD * (S(33) + S(121)) + S(17) + S(129) + TWO_3RD * (S(9)  + S(133)) + ONE_3RD * (S(5) + S(149)) + S(73))
#define SS_074 (s[ij] + FIVE_3RD * (S(0) + S(50)) + FOUR_3RD * (S(34) + S(120)) + S(18) + S(128) + TWO_3RD * (S(10) + S(132)) + ONE_3RD * (S(6) + S(148)) + S(74))
#define SS_075 (s[ij] + FIVE_3RD * (S(1) + S(51)) + FOUR_3RD * (S(35) + S(121)) + S(19) + S(129) + TWO_3RD * (S(11) + S(133)) + ONE_3RD * (S(7) + S(149)) + S(75))
#define SS_076 (s[ij] + FIVE_3RD * (S(2) + S(52)) + FOUR_3RD * (S(36) + S(122)) + S(20) + S(130) + TWO_3RD * (S(12) + S(134)) + ONE_3RD * (S(4) + S(150)) + S(76))
#define SS_077 (s[ij] + FIVE_3RD * (S(3) + S(53)) + FOUR_3RD * (S(37) + S(123)) + S(21) + S(131) + TWO_3RD * (S(13) + S(135)) + ONE_3RD * (S(5) + S(151)) + S(77))
#define SS_078 (s[ij] + FIVE_3RD * (S(3) + S(54)) + FOUR_3RD * (S(38) + S(123)) + S(22) + S(131) + TWO_3RD * (S(14) + S(135)) + ONE_3RD * (S(6) + S(151)) + S(78))
#define SS_079 (s[ij] + FIVE_3RD * (S(2) + S(55)) + FOUR_3RD * (S(39) + S(122)) + S(23) + S(130) + TWO_3RD * (S(15) + S(134)) + ONE_3RD * (S(7) + S(150)) + S(79))

	/* Do sqrt(41) nodes */

#define SS_080 (s[ij] + 0.25 * (S(0) + S(152)) + 0.75 * (S(8)  + S(136)) + 1.25 * (S(124) + S(24)) + 1.75 * (S(4) + S(40)) + S(80))
#define SS_081 (s[ij] + 0.25 * (S(1) + S(153)) + 0.75 * (S(9)  + S(137)) + 1.25 * (S(125) + S(25)) + 1.75 * (S(5) + S(41)) + S(81))
#define SS_082 (s[ij] + 0.25 * (S(0) + S(154)) + 0.75 * (S(10) + S(138)) + 1.25 * (S(126) + S(26)) + 1.75 * (S(6) + S(42)) + S(82))
#define SS_083 (s[ij] + 0.25 * (S(1) + S(155)) + 0.75 * (S(11) + S(139)) + 1.25 * (S(127) + S(27)) + 1.75 * (S(7) + S(43)) + S(83))
#define SS_084 (s[ij] + 0.25 * (S(2) + S(152)) + 0.75 * (S(12) + S(136)) + 1.25 * (S(124) + S(28)) + 1.75 * (S(4) + S(44)) + S(84))
#define SS_085 (s[ij] + 0.25 * (S(3) + S(153)) + 0.75 * (S(13) + S(137)) + 1.25 * (S(125) + S(29)) + 1.75 * (S(5) + S(45)) + S(85))
#define SS_086 (s[ij] + 0.25 * (S(3) + S(154)) + 0.75 * (S(14) + S(138)) + 1.25 * (S(126) + S(30)) + 1.75 * (S(6) + S(46)) + S(86))
#define SS_087 (s[ij] + 0.25 * (S(2) + S(155)) + 0.75 * (S(15) + S(139)) + 1.25 * (S(127) + S(31)) + 1.75 * (S(7) + S(47)) + S(87))

	/* Do sqrt(58) nodes */

#define SS_088 (s[ij] + EIGHT_7TH * (S(0) + S(168)) + SIX_7TH * (S(4) + S(160)) + THIRTY8_21ST * (S(8)  + S(56)) + FOUR_21ST * (S(120) + S(64)) + THIRTY2_21ST * (S(16) + S(140)) + TEN_21ST * (S(24) + S(32)) + S(88))
#define SS_089 (s[ij] + EIGHT_7TH * (S(1) + S(169)) + SIX_7TH * (S(5) + S(161)) + THIRTY8_21ST * (S(9)  + S(57)) + FOUR_21ST * (S(121) + S(65)) + THIRTY2_21ST * (S(17) + S(141)) + TEN_21ST * (S(25) + S(33)) + S(89))
#define SS_090 (s[ij] + EIGHT_7TH * (S(0) + S(170)) + SIX_7TH * (S(6) + S(162)) + THIRTY8_21ST * (S(10) + S(58)) + FOUR_21ST * (S(120) + S(66)) + THIRTY2_21ST * (S(18) + S(142)) + TEN_21ST * (S(26) + S(34)) + S(90))
#define SS_091 (s[ij] + EIGHT_7TH * (S(1) + S(171)) + SIX_7TH * (S(7) + S(163)) + THIRTY8_21ST * (S(11) + S(59)) + FOUR_21ST * (S(121) + S(67)) + THIRTY2_21ST * (S(19) + S(143)) + TEN_21ST * (S(27) + S(35)) + S(91))
#define SS_092 (s[ij] + EIGHT_7TH * (S(2) + S(172)) + SIX_7TH * (S(4) + S(164)) + THIRTY8_21ST * (S(12) + S(60)) + FOUR_21ST * (S(122) + S(68)) + THIRTY2_21ST * (S(20) + S(144)) + TEN_21ST * (S(28) + S(36)) + S(92))
#define SS_093 (s[ij] + EIGHT_7TH * (S(3) + S(173)) + SIX_7TH * (S(5) + S(165)) + THIRTY8_21ST * (S(13) + S(61)) + FOUR_21ST * (S(123) + S(69)) + THIRTY2_21ST * (S(21) + S(145)) + TEN_21ST * (S(29) + S(37)) + S(93))
#define SS_094 (s[ij] + EIGHT_7TH * (S(3) + S(174)) + SIX_7TH * (S(6) + S(166)) + THIRTY8_21ST * (S(14) + S(62)) + FOUR_21ST * (S(123) + S(70)) + THIRTY2_21ST * (S(22) + S(146)) + TEN_21ST * (S(30) + S(38)) + S(94))
#define SS_095 (s[ij] + EIGHT_7TH * (S(2) + S(175)) + SIX_7TH * (S(7) + S(167)) + THIRTY8_21ST * (S(15) + S(63)) + FOUR_21ST * (S(122) + S(71)) + THIRTY2_21ST * (S(23) + S(147)) + TEN_21ST * (S(31) + S(39)) + S(95))

	/* Do sqrt(65) nodes */

#define SS_096 (s[ij] + 1.75 * (S(0) + S(184)) + 1.5 * (S(120) + S(72)) + 1.25 * (S(128) + S(48)) + S(32) + S(132) + 0.75 * (S(16) + S(148)) + 0.5 * (S(8)  + S(156)) + 0.25 * (S(4) + S(176)) +  S(96))
#define SS_097 (s[ij] + 1.75 * (S(1) + S(185)) + 1.5 * (S(121) + S(73)) + 1.25 * (S(129) + S(49)) + S(33) + S(133) + 0.75 * (S(17) + S(149)) + 0.5 * (S(9)  + S(157)) + 0.25 * (S(5) + S(177)) +  S(97))
#define SS_098 (s[ij] + 1.75 * (S(0) + S(186)) + 1.5 * (S(120) + S(74)) + 1.25 * (S(128) + S(50)) + S(34) + S(132) + 0.75 * (S(18) + S(148)) + 0.5 * (S(10) + S(156)) + 0.25 * (S(6) + S(176)) +  S(98))
#define SS_099 (s[ij] + 1.75 * (S(1) + S(187)) + 1.5 * (S(121) + S(75)) + 1.25 * (S(129) + S(51)) + S(35) + S(133) + 0.75 * (S(19) + S(149)) + 0.5 * (S(11) + S(157)) + 0.25 * (S(7) + S(177)) +  S(99))
#define SS_100 (s[ij] + 1.75 * (S(2) + S(188)) + 1.5 * (S(122) + S(76)) + 1.25 * (S(130) + S(52)) + S(36) + S(134) + 0.75 * (S(20) + S(150)) + 0.5 * (S(12) + S(158)) + 0.25 * (S(4) + S(178)) + S(100))
#define SS_101 (s[ij] + 1.75 * (S(3) + S(189)) + 1.5 * (S(123) + S(77)) + 1.25 * (S(131) + S(53)) + S(37) + S(135) + 0.75 * (S(21) + S(151)) + 0.5 * (S(13) + S(159)) + 0.25 * (S(5) + S(179)) + S(101))
#define SS_102 (s[ij] + 1.75 * (S(3) + S(190)) + 1.5 * (S(123) + S(78)) + 1.25 * (S(131) + S(54)) + S(38) + S(135) + 0.75 * (S(22) + S(151)) + 0.5 * (S(14) + S(159)) + 0.25 * (S(6) + S(179)) + S(102))
#define SS_103 (s[ij] + 1.75 * (S(2) + S(191)) + 1.5 * (S(122) + S(79)) + 1.25 * (S(130) + S(55)) + S(39) + S(134) + 0.75 * (S(23) + S(150)) + 0.5 * (S(15) + S(158)) + 0.25 * (S(7) + S(178)) + S(103))

	/* Do sqrt(85) nodes */

#define SS_104 (s[ij] + ONE_6TH * (S(0) + S(204)) + ELEVEN_6TH * (S(4) + S(192)) + 0.5 * (S(8)  + S(180)) + 1.5 * (S(124) + S(80)) + FIVE_6TH * (S(24) + S(152)) + SEVEN_6TH * (S(136) + S(40)) + S(104))
#define SS_105 (s[ij] + ONE_6TH * (S(1) + S(205)) + ELEVEN_6TH * (S(5) + S(193)) + 0.5 * (S(9)  + S(181)) + 1.5 * (S(125) + S(81)) + FIVE_6TH * (S(25) + S(153)) + SEVEN_6TH * (S(137) + S(41)) + S(105))
#define SS_106 (s[ij] + ONE_6TH * (S(0) + S(206)) + ELEVEN_6TH * (S(6) + S(194)) + 0.5 * (S(10) + S(182)) + 1.5 * (S(126) + S(82)) + FIVE_6TH * (S(26) + S(154)) + SEVEN_6TH * (S(138) + S(42)) + S(106))
#define SS_107 (s[ij] + ONE_6TH * (S(1) + S(207)) + ELEVEN_6TH * (S(7) + S(195)) + 0.5 * (S(11) + S(183)) + 1.5 * (S(127) + S(83)) + FIVE_6TH * (S(27) + S(155)) + SEVEN_6TH * (S(139) + S(43)) + S(107))
#define SS_108 (s[ij] + ONE_6TH * (S(2) + S(204)) + ELEVEN_6TH * (S(4) + S(196)) + 0.5 * (S(12) + S(180)) + 1.5 * (S(124) + S(84)) + FIVE_6TH * (S(28) + S(152)) + SEVEN_6TH * (S(136) + S(44)) + S(108))
#define SS_109 (s[ij] + ONE_6TH * (S(3) + S(205)) + ELEVEN_6TH * (S(5) + S(197)) + 0.5 * (S(13) + S(181)) + 1.5 * (S(125) + S(85)) + FIVE_6TH * (S(29) + S(153)) + SEVEN_6TH * (S(137) + S(45)) + S(109))
#define SS_110 (s[ij] + ONE_6TH * (S(3) + S(206)) + ELEVEN_6TH * (S(6) + S(198)) + 0.5 * (S(14) + S(182)) + 1.5 * (S(126) + S(86)) + FIVE_6TH * (S(30) + S(154)) + SEVEN_6TH * (S(138) + S(46)) + S(110))
#define SS_111 (s[ij] + ONE_6TH * (S(2) + S(207)) + ELEVEN_6TH * (S(7) + S(199)) + 0.5 * (S(15) + S(183)) + 1.5 * (S(127) + S(87)) + FIVE_6TH * (S(31) + S(155)) + SEVEN_6TH * (S(139) + S(47)) + S(111))

	/* Do sqrt(170) nodes */

#define SS_112 (s[ij] + TWENTY4_13TH * (S(0) + S(248)) + TWENTY2_13TH * (S(120) + S(236)) + TWENTY_13TH * (S(128) + S(224)) + EIGHTEEN_13TH * (S(132) + S(212)) + SIXTEEN_13TH * (S(148) +  S(96)) + FOURTEEN_13TH * (S(156) + S(184)) + TWELVE_13TH * (S(176) + S(72)) + TEN_13TH * (S(200) + S(48)) + EIGHT_13TH * (S(208) + S(32)) + SIX_13TH * (S(220) + S(16)) + FOUR_13TH * (S(232) +  S(8)) + TWO_13TH * (S(244) + S(4)) + S(112))
#define SS_113 (s[ij] + TWENTY4_13TH * (S(1) + S(249)) + TWENTY2_13TH * (S(121) + S(237)) + TWENTY_13TH * (S(129) + S(225)) + EIGHTEEN_13TH * (S(133) + S(213)) + SIXTEEN_13TH * (S(149) +  S(97)) + FOURTEEN_13TH * (S(157) + S(185)) + TWELVE_13TH * (S(177) + S(73)) + TEN_13TH * (S(201) + S(49)) + EIGHT_13TH * (S(209) + S(33)) + SIX_13TH * (S(221) + S(17)) + FOUR_13TH * (S(233) +  S(9)) + TWO_13TH * (S(245) + S(5)) + S(113))
#define SS_114 (s[ij] + TWENTY4_13TH * (S(0) + S(250)) + TWENTY2_13TH * (S(120) + S(238)) + TWENTY_13TH * (S(128) + S(226)) + EIGHTEEN_13TH * (S(132) + S(214)) + SIXTEEN_13TH * (S(148) +  S(98)) + FOURTEEN_13TH * (S(156) + S(186)) + TWELVE_13TH * (S(176) + S(74)) + TEN_13TH * (S(200) + S(50)) + EIGHT_13TH * (S(208) + S(34)) + SIX_13TH * (S(220) + S(18)) + FOUR_13TH * (S(232) + S(10)) + TWO_13TH * (S(244) + S(6)) + S(114))
#define SS_115 (s[ij] + TWENTY4_13TH * (S(1) + S(251)) + TWENTY2_13TH * (S(121) + S(239)) + TWENTY_13TH * (S(129) + S(227)) + EIGHTEEN_13TH * (S(133) + S(215)) + SIXTEEN_13TH * (S(149) +  S(99)) + FOURTEEN_13TH * (S(157) + S(187)) + TWELVE_13TH * (S(177) + S(75)) + TEN_13TH * (S(201) + S(51)) + EIGHT_13TH * (S(209) + S(35)) + SIX_13TH * (S(221) + S(19)) + FOUR_13TH * (S(233) + S(11)) + TWO_13TH * (S(245) + S(7)) + S(115))
#define SS_116 (s[ij] + TWENTY4_13TH * (S(2) + S(252)) + TWENTY2_13TH * (S(122) + S(240)) + TWENTY_13TH * (S(130) + S(228)) + EIGHTEEN_13TH * (S(134) + S(216)) + SIXTEEN_13TH * (S(150) + S(100)) + FOURTEEN_13TH * (S(158) + S(188)) + TWELVE_13TH * (S(178) + S(76)) + TEN_13TH * (S(202) + S(52)) + EIGHT_13TH * (S(210) + S(36)) + SIX_13TH * (S(222) + S(20)) + FOUR_13TH * (S(234) + S(12)) + TWO_13TH * (S(246) + S(4)) + S(116))
#define SS_117 (s[ij] + TWENTY4_13TH * (S(3) + S(253)) + TWENTY2_13TH * (S(123) + S(241)) + TWENTY_13TH * (S(131) + S(229)) + EIGHTEEN_13TH * (S(135) + S(217)) + SIXTEEN_13TH * (S(151) + S(101)) + FOURTEEN_13TH * (S(159) + S(189)) + TWELVE_13TH * (S(179) + S(77)) + TEN_13TH * (S(203) + S(53)) + EIGHT_13TH * (S(211) + S(37)) + SIX_13TH * (S(223) + S(21)) + FOUR_13TH * (S(235) + S(13)) + TWO_13TH * (S(247) + S(5)) + S(117))
#define SS_118 (s[ij] + TWENTY4_13TH * (S(3) + S(254)) + TWENTY2_13TH * (S(123) + S(242)) + TWENTY_13TH * (S(131) + S(230)) + EIGHTEEN_13TH * (S(135) + S(218)) + SIXTEEN_13TH * (S(151) + S(102)) + FOURTEEN_13TH * (S(159) + S(190)) + TWELVE_13TH * (S(179) + S(78)) + TEN_13TH * (S(203) + S(54)) + EIGHT_13TH * (S(211) + S(38)) + SIX_13TH * (S(223) + S(22)) + FOUR_13TH * (S(235) + S(14)) + TWO_13TH * (S(247) + S(6)) + S(118))
#define SS_119 (s[ij] + TWENTY4_13TH * (S(2) + S(255)) + TWENTY2_13TH * (S(122) + S(243)) + TWENTY_13TH * (S(130) + S(231)) + EIGHTEEN_13TH * (S(134) + S(219)) + SIXTEEN_13TH * (S(150) + S(103)) + FOURTEEN_13TH * (S(158) + S(191)) + TWELVE_13TH * (S(178) + S(79)) + TEN_13TH * (S(202) + S(55)) + EIGHT_13TH * (S(210) + S(39)) + SIX_13TH * (S(222) + S(23)) + FOUR_13TH * (S(234) + S(15)) + TWO_13TH * (S(246) + S(7)) + S(119))
