#ifndef HTCW_GFX_COLOR_HPP
#define HTCP_GFX_COLOR_HPP
#include "gfx_pixel.hpp"
namespace gfx {
    // predefined color values
    template<typename PixelType>
    class color final {
        // we use a super precise max-bit RGB pixel for this
        using source_type = rgb_pixel<HTCW_MAX_WORD>;
    public:
        constexpr static const PixelType alice_blue = source_type(true,0.941176470588235,0.972549019607843,1).convert<PixelType>();
        constexpr static const PixelType antique_white = source_type(true,0.980392156862745,0.92156862745098,0.843137254901961).convert<PixelType>();
        constexpr static const PixelType aqua = source_type(true,0,1,1).convert<PixelType>();
        constexpr static const PixelType aquamarine = source_type(true,0.498039215686275,1,0.831372549019608).convert<PixelType>();
        constexpr static const PixelType azure = source_type(true,0.941176470588235,1,1).convert<PixelType>();
        constexpr static const PixelType beige = source_type(true,0.96078431372549,0.96078431372549,0.862745098039216).convert<PixelType>();
        constexpr static const PixelType bisque = source_type(true,1,0.894117647058824,0.768627450980392).convert<PixelType>();
        constexpr static const PixelType black = source_type(true,0,0,0).convert<PixelType>();
        constexpr static const PixelType blanched_almond = source_type(true,1,0.92156862745098,0.803921568627451).convert<PixelType>();
        constexpr static const PixelType blue = source_type(true,0,0,1).convert<PixelType>();
        constexpr static const PixelType blue_violet = source_type(true,0.541176470588235,0.168627450980392,0.886274509803922).convert<PixelType>();
        constexpr static const PixelType brown = source_type(true,0.647058823529412,0.164705882352941,0.164705882352941).convert<PixelType>();
        constexpr static const PixelType burly_wood = source_type(true,0.870588235294118,0.72156862745098,0.529411764705882).convert<PixelType>();
        constexpr static const PixelType cadet_blue = source_type(true,0.372549019607843,0.619607843137255,0.627450980392157).convert<PixelType>();
        constexpr static const PixelType chartreuse = source_type(true,0.498039215686275,1,0).convert<PixelType>();
        constexpr static const PixelType chocolate = source_type(true,0.823529411764706,0.411764705882353,0.117647058823529).convert<PixelType>();
        constexpr static const PixelType coral = source_type(true,1,0.498039215686275,0.313725490196078).convert<PixelType>();
        constexpr static const PixelType cornflower_blue = source_type(true,0.392156862745098,0.584313725490196,0.929411764705882).convert<PixelType>();
        constexpr static const PixelType cornsilk = source_type(true,1,0.972549019607843,0.862745098039216).convert<PixelType>();
        constexpr static const PixelType crimson = source_type(true,0.862745098039216,0.0784313725490196,0.235294117647059).convert<PixelType>();
        constexpr static const PixelType cyan = source_type(true,0,1,1).convert<PixelType>();
        constexpr static const PixelType dark_blue = source_type(true,0,0,0.545098039215686).convert<PixelType>();
        constexpr static const PixelType dark_cyan = source_type(true,0,0.545098039215686,0.545098039215686).convert<PixelType>();
        constexpr static const PixelType dark_goldenrod = source_type(true,0.72156862745098,0.525490196078431,0.0431372549019608).convert<PixelType>();
        constexpr static const PixelType dark_gray = source_type(true,0.662745098039216,0.662745098039216,0.662745098039216).convert<PixelType>();
        constexpr static const PixelType dark_green = source_type(true,0,0.392156862745098,0).convert<PixelType>();
        constexpr static const PixelType dark_khaki = source_type(true,0.741176470588235,0.717647058823529,0.419607843137255).convert<PixelType>();
        constexpr static const PixelType dark_magenta = source_type(true,0.545098039215686,0,0.545098039215686).convert<PixelType>();
        constexpr static const PixelType dark_olive_green = source_type(true,0.333333333333333,0.419607843137255,0.184313725490196).convert<PixelType>();
        constexpr static const PixelType dark_orange = source_type(true,1,0.549019607843137,0).convert<PixelType>();
        constexpr static const PixelType dark_orchid = source_type(true,0.6,0.196078431372549,0.8).convert<PixelType>();
        constexpr static const PixelType dark_red = source_type(true,0.545098039215686,0,0).convert<PixelType>();
        constexpr static const PixelType dark_salmon = source_type(true,0.913725490196078,0.588235294117647,0.47843137254902).convert<PixelType>();
        constexpr static const PixelType dark_sea_green = source_type(true,0.56078431372549,0.737254901960784,0.545098039215686).convert<PixelType>();
        constexpr static const PixelType dark_slate_blue = source_type(true,0.282352941176471,0.23921568627451,0.545098039215686).convert<PixelType>();
        constexpr static const PixelType dark_slate_gray = source_type(true,0.184313725490196,0.309803921568627,0.309803921568627).convert<PixelType>();
        constexpr static const PixelType dark_turquoise = source_type(true,0,0.807843137254902,0.819607843137255).convert<PixelType>();
        constexpr static const PixelType dark_violet = source_type(true,0.580392156862745,0,0.827450980392157).convert<PixelType>();
        constexpr static const PixelType deep_pink = source_type(true,1,0.0784313725490196,0.576470588235294).convert<PixelType>();
        constexpr static const PixelType deep_sky_blue = source_type(true,0,0.749019607843137,1).convert<PixelType>();
        constexpr static const PixelType dim_gray = source_type(true,0.411764705882353,0.411764705882353,0.411764705882353).convert<PixelType>();
        constexpr static const PixelType dodger_blue = source_type(true,0.117647058823529,0.564705882352941,1).convert<PixelType>();
        constexpr static const PixelType firebrick = source_type(true,0.698039215686274,0.133333333333333,0.133333333333333).convert<PixelType>();
        constexpr static const PixelType floral_white = source_type(true,1,0.980392156862745,0.941176470588235).convert<PixelType>();
        constexpr static const PixelType forest_green = source_type(true,0.133333333333333,0.545098039215686,0.133333333333333).convert<PixelType>();
        constexpr static const PixelType fuchsia = source_type(true,1,0,1).convert<PixelType>();
        constexpr static const PixelType gainsboro = source_type(true,0.862745098039216,0.862745098039216,0.862745098039216).convert<PixelType>();
        constexpr static const PixelType ghost_white = source_type(true,0.972549019607843,0.972549019607843,1).convert<PixelType>();
        constexpr static const PixelType gold = source_type(true,1,0.843137254901961,0).convert<PixelType>();
        constexpr static const PixelType goldenrod = source_type(true,0.854901960784314,0.647058823529412,0.125490196078431).convert<PixelType>();
        constexpr static const PixelType gray = source_type(true,0.501960784313725,0.501960784313725,0.501960784313725).convert<PixelType>();
        constexpr static const PixelType green = source_type(true,0,0.501960784313725,0).convert<PixelType>();
        constexpr static const PixelType green_yellow = source_type(true,0.67843137254902,1,0.184313725490196).convert<PixelType>();
        constexpr static const PixelType honeydew = source_type(true,0.941176470588235,1,0.941176470588235).convert<PixelType>();
        constexpr static const PixelType hot_pink = source_type(true,1,0.411764705882353,0.705882352941177).convert<PixelType>();
        constexpr static const PixelType indian_red = source_type(true,0.803921568627451,0.36078431372549,0.36078431372549).convert<PixelType>();
        constexpr static const PixelType indigo = source_type(true,0.294117647058824,0,0.509803921568627).convert<PixelType>();
        constexpr static const PixelType ivory = source_type(true,1,1,0.941176470588235).convert<PixelType>();
        constexpr static const PixelType khaki = source_type(true,0.941176470588235,0.901960784313726,0.549019607843137).convert<PixelType>();
        constexpr static const PixelType lavender = source_type(true,0.901960784313726,0.901960784313726,0.980392156862745).convert<PixelType>();
        constexpr static const PixelType lavender_blush = source_type(true,1,0.941176470588235,0.96078431372549).convert<PixelType>();
        constexpr static const PixelType lawn_green = source_type(true,0.486274509803922,0.988235294117647,0).convert<PixelType>();
        constexpr static const PixelType lemon_chiffon = source_type(true,1,0.980392156862745,0.803921568627451).convert<PixelType>();
        constexpr static const PixelType light_blue = source_type(true,0.67843137254902,0.847058823529412,0.901960784313726).convert<PixelType>();
        constexpr static const PixelType light_coral = source_type(true,0.941176470588235,0.501960784313725,0.501960784313725).convert<PixelType>();
        constexpr static const PixelType light_cyan = source_type(true,0.87843137254902,1,1).convert<PixelType>();
        constexpr static const PixelType light_goldenrod_yellow = source_type(true,0.980392156862745,0.980392156862745,0.823529411764706).convert<PixelType>();
        constexpr static const PixelType light_green = source_type(true,0.564705882352941,0.933333333333333,0.564705882352941).convert<PixelType>();
        constexpr static const PixelType light_gray = source_type(true,0.827450980392157,0.827450980392157,0.827450980392157).convert<PixelType>();
        constexpr static const PixelType light_pink = source_type(true,1,0.713725490196078,0.756862745098039).convert<PixelType>();
        constexpr static const PixelType light_salmon = source_type(true,1,0.627450980392157,0.47843137254902).convert<PixelType>();
        constexpr static const PixelType light_sea_green = source_type(true,0.125490196078431,0.698039215686274,0.666666666666667).convert<PixelType>();
        constexpr static const PixelType light_sky_blue = source_type(true,0.529411764705882,0.807843137254902,0.980392156862745).convert<PixelType>();
        constexpr static const PixelType light_slate_gray = source_type(true,0.466666666666667,0.533333333333333,0.6).convert<PixelType>();
        constexpr static const PixelType light_steel_blue = source_type(true,0.690196078431373,0.768627450980392,0.870588235294118).convert<PixelType>();
        constexpr static const PixelType light_yellow = source_type(true,1,1,0.87843137254902).convert<PixelType>();
        constexpr static const PixelType lime = source_type(true,0,1,0).convert<PixelType>();
        constexpr static const PixelType lime_green = source_type(true,0.196078431372549,0.803921568627451,0.196078431372549).convert<PixelType>();
        constexpr static const PixelType linen = source_type(true,0.980392156862745,0.941176470588235,0.901960784313726).convert<PixelType>();
        constexpr static const PixelType magenta = source_type(true,1,0,1).convert<PixelType>();
        constexpr static const PixelType maroon = source_type(true,0.501960784313725,0,0).convert<PixelType>();
        constexpr static const PixelType medium_aquamarine = source_type(true,0.4,0.803921568627451,0.666666666666667).convert<PixelType>();
        constexpr static const PixelType medium_blue = source_type(true,0,0,0.803921568627451).convert<PixelType>();
        constexpr static const PixelType medium_orchid = source_type(true,0.729411764705882,0.333333333333333,0.827450980392157).convert<PixelType>();
        constexpr static const PixelType medium_purple = source_type(true,0.576470588235294,0.43921568627451,0.858823529411765).convert<PixelType>();
        constexpr static const PixelType medium_sea_green = source_type(true,0.235294117647059,0.701960784313725,0.443137254901961).convert<PixelType>();
        constexpr static const PixelType medium_slate_blue = source_type(true,0.482352941176471,0.407843137254902,0.933333333333333).convert<PixelType>();
        constexpr static const PixelType medium_spring_green = source_type(true,0,0.980392156862745,0.603921568627451).convert<PixelType>();
        constexpr static const PixelType medium_turquoise = source_type(true,0.282352941176471,0.819607843137255,0.8).convert<PixelType>();
        constexpr static const PixelType medium_violet_red = source_type(true,0.780392156862745,0.0823529411764706,0.52156862745098).convert<PixelType>();
        constexpr static const PixelType midnight_blue = source_type(true,0.0980392156862745,0.0980392156862745,0.43921568627451).convert<PixelType>();
        constexpr static const PixelType mint_cream = source_type(true,0.96078431372549,1,0.980392156862745).convert<PixelType>();
        constexpr static const PixelType misty_rose = source_type(true,1,0.894117647058824,0.882352941176471).convert<PixelType>();
        constexpr static const PixelType moccasin = source_type(true,1,0.894117647058824,0.709803921568627).convert<PixelType>();
        constexpr static const PixelType navajo_white = source_type(true,1,0.870588235294118,0.67843137254902).convert<PixelType>();
        constexpr static const PixelType navy = source_type(true,0,0,0.501960784313725).convert<PixelType>();
        constexpr static const PixelType old_lace = source_type(true,0.992156862745098,0.96078431372549,0.901960784313726).convert<PixelType>();
        constexpr static const PixelType olive = source_type(true,0.501960784313725,0.501960784313725,0).convert<PixelType>();
        constexpr static const PixelType olive_drab = source_type(true,0.419607843137255,0.556862745098039,0.137254901960784).convert<PixelType>();
        constexpr static const PixelType orange = source_type(true,1,0.647058823529412,0).convert<PixelType>();
        constexpr static const PixelType orange_red = source_type(true,1,0.270588235294118,0).convert<PixelType>();
        constexpr static const PixelType orchid = source_type(true,0.854901960784314,0.43921568627451,0.83921568627451).convert<PixelType>();
        constexpr static const PixelType pale_goldenrod = source_type(true,0.933333333333333,0.909803921568627,0.666666666666667).convert<PixelType>();
        constexpr static const PixelType pale_green = source_type(true,0.596078431372549,0.984313725490196,0.596078431372549).convert<PixelType>();
        constexpr static const PixelType pale_turquoise = source_type(true,0.686274509803922,0.933333333333333,0.933333333333333).convert<PixelType>();
        constexpr static const PixelType pale_violet_red = source_type(true,0.858823529411765,0.43921568627451,0.576470588235294).convert<PixelType>();
        constexpr static const PixelType papaya_whip = source_type(true,1,0.937254901960784,0.835294117647059).convert<PixelType>();
        constexpr static const PixelType peach_puff = source_type(true,1,0.854901960784314,0.725490196078431).convert<PixelType>();
        constexpr static const PixelType peru = source_type(true,0.803921568627451,0.52156862745098,0.247058823529412).convert<PixelType>();
        constexpr static const PixelType pink = source_type(true,1,0.752941176470588,0.796078431372549).convert<PixelType>();
        constexpr static const PixelType plum = source_type(true,0.866666666666667,0.627450980392157,0.866666666666667).convert<PixelType>();
        constexpr static const PixelType powder_blue = source_type(true,0.690196078431373,0.87843137254902,0.901960784313726).convert<PixelType>();
        constexpr static const PixelType purple = source_type(true,0.501960784313725,0,0.501960784313725).convert<PixelType>();
        constexpr static const PixelType red = source_type(true,1,0,0).convert<PixelType>();
        constexpr static const PixelType rosy_brown = source_type(true,0.737254901960784,0.56078431372549,0.56078431372549).convert<PixelType>();
        constexpr static const PixelType royal_blue = source_type(true,0.254901960784314,0.411764705882353,0.882352941176471).convert<PixelType>();
        constexpr static const PixelType saddle_brown = source_type(true,0.545098039215686,0.270588235294118,0.0745098039215686).convert<PixelType>();
        constexpr static const PixelType salmon = source_type(true,0.980392156862745,0.501960784313725,0.447058823529412).convert<PixelType>();
        constexpr static const PixelType sandy_brown = source_type(true,0.956862745098039,0.643137254901961,0.376470588235294).convert<PixelType>();
        constexpr static const PixelType sea_green = source_type(true,0.180392156862745,0.545098039215686,0.341176470588235).convert<PixelType>();
        constexpr static const PixelType sea_shell = source_type(true,1,0.96078431372549,0.933333333333333).convert<PixelType>();
        constexpr static const PixelType sienna = source_type(true,0.627450980392157,0.32156862745098,0.176470588235294).convert<PixelType>();
        constexpr static const PixelType silver = source_type(true,0.752941176470588,0.752941176470588,0.752941176470588).convert<PixelType>();
        constexpr static const PixelType sky_blue = source_type(true,0.529411764705882,0.807843137254902,0.92156862745098).convert<PixelType>();
        constexpr static const PixelType slate_blue = source_type(true,0.415686274509804,0.352941176470588,0.803921568627451).convert<PixelType>();
        constexpr static const PixelType slate_gray = source_type(true,0.43921568627451,0.501960784313725,0.564705882352941).convert<PixelType>();
        constexpr static const PixelType snow = source_type(true,1,0.980392156862745,0.980392156862745).convert<PixelType>();
        constexpr static const PixelType spring_green = source_type(true,0,1,0.498039215686275).convert<PixelType>();
        constexpr static const PixelType steel_blue = source_type(true,0.274509803921569,0.509803921568627,0.705882352941177).convert<PixelType>();
        constexpr static const PixelType tan = source_type(true,0.823529411764706,0.705882352941177,0.549019607843137).convert<PixelType>();
        constexpr static const PixelType teal = source_type(true,0,0.501960784313725,0.501960784313725).convert<PixelType>();
        constexpr static const PixelType thistle = source_type(true,0.847058823529412,0.749019607843137,0.847058823529412).convert<PixelType>();
        constexpr static const PixelType tomato = source_type(true,1,0.388235294117647,0.27843137254902).convert<PixelType>();
        constexpr static const PixelType turquoise = source_type(true,0.250980392156863,0.87843137254902,0.815686274509804).convert<PixelType>();
        constexpr static const PixelType violet = source_type(true,0.933333333333333,0.509803921568627,0.933333333333333).convert<PixelType>();
        constexpr static const PixelType wheat = source_type(true,0.96078431372549,0.870588235294118,0.701960784313725).convert<PixelType>();
        constexpr static const PixelType white = source_type(true,1,1,1).convert<PixelType>();
        constexpr static const PixelType white_smoke = source_type(true,0.96078431372549,0.96078431372549,0.96078431372549).convert<PixelType>();
        constexpr static const PixelType yellow = source_type(true,1,1,0).convert<PixelType>();
        constexpr static const PixelType yellow_green = source_type(true,0.603921568627451,0.803921568627451,0.196078431372549).convert<PixelType>();
        // ensure colors can be converted to destination type:
        static_assert(0!=white.native_value || 0!=black.native_value,"Color type is not convertible from RGB");
    };
}
#endif