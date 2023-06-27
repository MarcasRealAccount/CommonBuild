#include "UTF/UTF.h"

namespace UTF
{
	CalcReqSizeImplF s_CalcReqSizeImpls[c_EncodingCount][c_EncodingCount][c_ImplCount];
	ConvBlockImplF   s_ConvBlockImpls[c_EncodingCount][c_EncodingCount][c_ImplCount];

	static struct Initializer
	{
		void SetFuncs(EEncoding from, EEncoding to, EImpl impl, CalcReqSizeImplF calcFunc, ConvBlockImplF convFunc)
		{
			s_CalcReqSizeImpls[static_cast<std::uint8_t>(from)][static_cast<std::uint8_t>(to)][static_cast<std::uint8_t>(impl)] = calcFunc;
			s_ConvBlockImpls[static_cast<std::uint8_t>(from)][static_cast<std::uint8_t>(to)][static_cast<std::uint8_t>(impl)]   = convFunc;
		}

		Initializer()
		{
			SetFuncs(EEncoding::UTF8, EEncoding::UTF8, EImpl::Generic, nullptr, nullptr);
			SetFuncs(EEncoding::UTF8, EEncoding::UTF8, EImpl::SIMD, nullptr, nullptr);
			SetFuncs(EEncoding::UTF8, EEncoding::UTF16, EImpl::Generic, &Generic::CalcReqSize8To16, &Generic::ConvBlock8To16);
			SetFuncs(EEncoding::UTF8, EEncoding::UTF16, EImpl::SIMD, &SIMD::CalcReqSize8To16, &SIMD::ConvBlock8To16);
			SetFuncs(EEncoding::UTF8, EEncoding::UTF32, EImpl::Generic, &Generic::CalcReqSize8To32, &Generic::ConvBlock8To32);
			SetFuncs(EEncoding::UTF8, EEncoding::UTF32, EImpl::SIMD, &SIMD::CalcReqSize8To32, &SIMD::ConvBlock8To32);
			SetFuncs(EEncoding::UTF16, EEncoding::UTF8, EImpl::Generic, &Generic::CalcReqSize16To8, &Generic::ConvBlock16To8);
			SetFuncs(EEncoding::UTF16, EEncoding::UTF8, EImpl::SIMD, &SIMD::CalcReqSize16To8, &SIMD::ConvBlock16To8);
			SetFuncs(EEncoding::UTF16, EEncoding::UTF16, EImpl::Generic, nullptr, nullptr);
			SetFuncs(EEncoding::UTF16, EEncoding::UTF16, EImpl::SIMD, nullptr, nullptr);
			SetFuncs(EEncoding::UTF16, EEncoding::UTF32, EImpl::Generic, &Generic::CalcReqSize16To32, &Generic::ConvBlock16To32);
			SetFuncs(EEncoding::UTF16, EEncoding::UTF32, EImpl::SIMD, &SIMD::CalcReqSize16To32, &SIMD::ConvBlock16To32);
			SetFuncs(EEncoding::UTF32, EEncoding::UTF8, EImpl::Generic, &Generic::CalcReqSize32To8, &Generic::ConvBlock32To8);
			SetFuncs(EEncoding::UTF32, EEncoding::UTF8, EImpl::SIMD, &SIMD::CalcReqSize32To8, &SIMD::ConvBlock32To8);
			SetFuncs(EEncoding::UTF32, EEncoding::UTF16, EImpl::Generic, &Generic::CalcReqSize32To16, &Generic::ConvBlock32To16);
			SetFuncs(EEncoding::UTF32, EEncoding::UTF16, EImpl::SIMD, &SIMD::CalcReqSize32To16, &SIMD::ConvBlock32To16);
			SetFuncs(EEncoding::UTF32, EEncoding::UTF32, EImpl::Generic, nullptr, nullptr);
			SetFuncs(EEncoding::UTF32, EEncoding::UTF32, EImpl::SIMD, nullptr, nullptr);
		}
	} s_Initializer;

	EImpl GetFastestImpl()
	{
		if constexpr (SIMD::c_Supported)
			return EImpl::SIMD;
		else
			return EImpl::Generic;
	}
} // namespace UTF