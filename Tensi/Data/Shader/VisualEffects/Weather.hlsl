//================================================.
//	天候・環境系の視覚効果シェーダまとめ.
//	雨 / 雪 / フォグ(ノイズミスト式) / 深度フォグ.
//================================================.

// テクスチャはレジスタ t(n).
Texture2D			g_SrcTexture	: register( t0 );	// 入力(直前の描画結果).
Texture2D			g_SubTexture	: register( t1 );	// 補助入力(未使用).
Texture2D			g_DepthTexture	: register( t2 );	// 深度バッファ(通常).
Texture2DMS<float>	g_DepthTextureMS: register( t3 );	// 深度バッファ(MSAA).
// サンプラはレジスタ s(n).
SamplerState		g_SamLinear		: register( s0 );

// 共通定数バッファ.
cbuffer Common : register( b0 )
{
	float4	g_Common0;	// x:画面幅, y:画面高さ, z:経過時間(秒), w:適用率(0～1).
	float4	g_Common1;	// x:透明部分の持ち上げアルファ, yzw:未使用.
	float4	g_Area;		// 適用範囲(px). 幅か高さが0以下で画面全体.
};
// エフェクト毎のパラメータ定数バッファ(内容は各エントリのコメントを参照).
cbuffer EffectParam : register( b1 )
{
	float4	g_P0;
	float4	g_P1;
	float4	g_P2;
	float4	g_P3;
};

// 頂点シェーダの出力パラメータ.
struct VS_OUTPUT
{
	float4	Pos	: SV_Position;	// 座標.
	float2	UV	: TEXCOORD0;	// UV座標.
};

//================================================.
//	共通関数.
//================================================.

// 頂点シェーダ(全画面三角形. 頂点バッファ不要).
VS_OUTPUT VS_Main( uint id : SV_VertexID )
{
	VS_OUTPUT o;
	float2 uv = float2( ( id << 1 ) & 2, id & 2 );
	o.Pos	= float4( uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, 0.0f, 1.0f );
	o.UV	= uv;
	return o;
}

// 適用範囲外か(ピクセル座標).
bool IsOutArea( float2 pos )
{
	if ( g_Area.z <= 0.0f || g_Area.w <= 0.0f ) return false;
	return ( pos.x <  g_Area.x || pos.y <  g_Area.y ||
			 pos.x >= g_Area.x + g_Area.z || pos.y >= g_Area.y + g_Area.w );
}

// 共通の仕上げ(元色との適用率合成と透明部分の持ち上げ).
float4 FinishColor( float4 src, float4 effect )
{
	float4 col = lerp( src, effect, g_Common0.w );
	col.a = max( col.a, g_Common1.x * g_Common0.w );
	return col;
}

// 乱数(0～1).
float Rand( float2 seed )
{
	return frac( sin( dot( seed, float2( 12.9898f, 78.233f ) ) ) * 43758.5453f );
}

// 値ノイズ(0～1. 滑らかに変化する).
float ValueNoise( float2 p )
{
	float2 i = floor( p );
	float2 f = frac( p );
	float2 u = f * f * ( 3.0f - 2.0f * f );
	float a = Rand( i );
	float b = Rand( i + float2( 1.0f, 0.0f ) );
	float c = Rand( i + float2( 0.0f, 1.0f ) );
	float d = Rand( i + float2( 1.0f, 1.0f ) );
	return lerp( lerp( a, b, u.x ), lerp( c, d, u.x ), u.y );
}

// 3オクターブの重ね合わせノイズ(0～1).
float Fbm( float2 p )
{
	float v = 0.0f;
	v += ValueNoise( p ) * 0.5f;
	v += ValueNoise( p * 2.03f ) * 0.25f;
	v += ValueNoise( p * 4.13f ) * 0.125f;
	return v / 0.875f;
}

//================================================.
//	すりガラス風の簡易ぼかし(9タップ).
//================================================.
float4 GlassBlur( float2 uv, float radiusPx )
{
	// 分岐後(IsOutArea の early-return より後)から呼ばれるため、
	// 微分を伴う Sample は使えない. ミップ固定の SampleLevel を使う.
	float2 p = radiusPx / g_Common0.xy;
	float4 c = g_SrcTexture.SampleLevel( g_SamLinear, uv, 0.0f ) * 0.25f;
	c += g_SrcTexture.SampleLevel( g_SamLinear, uv + float2(  p.x, 0.0f ), 0.0f ) * 0.125f;
	c += g_SrcTexture.SampleLevel( g_SamLinear, uv + float2( -p.x, 0.0f ), 0.0f ) * 0.125f;
	c += g_SrcTexture.SampleLevel( g_SamLinear, uv + float2( 0.0f,  p.y ), 0.0f ) * 0.125f;
	c += g_SrcTexture.SampleLevel( g_SamLinear, uv + float2( 0.0f, -p.y ), 0.0f ) * 0.125f;
	c += g_SrcTexture.SampleLevel( g_SamLinear, uv + p, 0.0f ) * 0.0625f;
	c += g_SrcTexture.SampleLevel( g_SamLinear, uv - p, 0.0f ) * 0.0625f;
	c += g_SrcTexture.SampleLevel( g_SamLinear, uv + float2(  p.x, -p.y ), 0.0f ) * 0.0625f;
	c += g_SrcTexture.SampleLevel( g_SamLinear, uv + float2( -p.x,  p.y ), 0.0f ) * 0.0625f;
	return c;
}

//================================================.
//	雨(窓ガラスを流れ落ちる雨滴).
//	列を流れ落ちる水滴と、その背後に残る細かい雫のトレイルを高さ場として生成し、
//	その勾配を法線として、ぼかしたガラス面(背景)を屈折させる.
//	水滴はすべて流れ落ちる(静止した水玉は無い).
//	g_P0 : x:雨の量(0～1), y:流れ落ちる速さ, z:屈折の強さ, w:ガラスのぼかし(px).
//	g_P1 : x:ハイライトの強さ.
//================================================.

// ハッシュ乱数(0～1).
float Hash21( float2 p )
{
	p = frac( p * float2( 234.34f, 435.345f ) );
	p += dot( p, p + 34.23f );
	return frac( p.x * p.y );
}

// のこぎり波(0→bで立ち上がり、b→1で立ち下がる).
float Saw( float b, float t )
{
	return smoothstep( 0.0f, b, t ) * smoothstep( 1.0f, b, t );
}

// 1層分の流れ落ちる水滴+トレイルの高さ場(BigWIngs "Heartfelt" 方式).
//	uv : アスペクト補正済み・上下反転済み(y-up)座標. t : 時間.
float DropLayer( float2 uv, float t )
{
	float2 UV = uv;
	uv.y += t * 0.75f;						// 時間で下へスクロール.
	float2 a = float2( 6.0f, 1.0f );		// セルの縦横比(縦長).
	float2 grid = a * 2.0f;
	float2 id = floor( uv * grid );
	float colShift = Hash21( float2( id.x, 0.0f ) );
	uv.y += colShift;						// 列ごとに位相をずらす.

	id = floor( uv * grid );
	float n = Hash21( id );
	float2 st = frac( uv * grid ) - float2( 0.5f, 0.0f );

	// 水滴の横位置(蛇行しながら流れる).
	float x = n - 0.5f;
	float y = UV.y * 20.0f;
	float wiggle = sin( y + sin( y ) );
	x += wiggle * ( 0.5f - abs( x ) ) * ( n - 0.5f );
	x *= 0.7f;
	// 水滴の縦位置(のこぎり波).
	//	振幅を下げて、のこぎり波の上昇成分がスクロール(下向き)を超えないようにする.
	//	→ 画面上では「上に上がる」ことがなくなり、ゆっくり溜まる(止まる)→スッと落ちるだけになる.
	//	振幅は水滴ごとにランダムにして、あまり止まらずスッと落ちる粒と、溜まってから落ちる粒を混ぜる.
	float ti = frac( t + n );
	float amp = 0.3f + 0.35f * Hash21( id + 19.0f );
	y = ( Saw( 0.85f, ti ) - 0.5f ) * amp + 0.5f;
	float2 p = float2( x, y );

	// 本体の水滴.
	float d = length( ( st - p ) * a.yx );
	float mainDrop = smoothstep( 0.4f, 0.0f, d );

	// トレイル(水滴が通った後の細い濡れ跡=水滴より上側).
	float r = sqrt( smoothstep( 1.0f, y, st.y ) );
	float cd = abs( st.x - x );
	float trail = smoothstep( 0.23f * r, 0.15f * r * r, cd );
	float trailFront = smoothstep( -0.02f, 0.02f, st.y - y );
	trail *= trailFront * r * r;

	// トレイルに残る小さな雫.
	y = UV.y;
	float trail2 = smoothstep( 0.2f * r, 0.0f, cd );
	float droplets = max( 0.0f, ( sin( y * ( 1.0f - y ) * 120.0f ) - st.y ) ) * trail2 * trailFront * n;
	y = frac( y * 10.0f ) + ( st.y - 0.5f );
	float dd = length( st - float2( x, y ) );
	droplets = smoothstep( 0.3f, 0.0f, dd );

	return mainDrop + droplets * r * trailFront;
}

// 2層を重ねた雨の高さ場.
float RainHeight( float2 uv, float t, float amount )
{
	float c = DropLayer( uv, t );					// 大きめの水滴.
	c += DropLayer( uv * 1.85f + 3.7f, t );			// 小さめの水滴.
	return c * amount;
}

float4 PS_Rain( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float aspect = g_Common0.x / max( g_Common0.y, 1.0f );
	float amount = saturate( g_P0.x );					// 雨の量.
	float t = g_Common0.z * g_P0.y;

	// アスペクト補正 + 上下反転.
	//	ShaderToy系(BigWIngs)はY軸が上向きだが D3D のUVは下が正なので、
	//	反転して水滴が下へ流れ落ちるようにする.
	float2 fuv = float2( i.UV.x * aspect, 1.0f - i.UV.y );

	// 高さ場を3点評価して勾配(法線)を求める( 微分命令を使わない ).
	float2 e = float2( 0.0016f, 0.0f );
	float c  = RainHeight( fuv,        t, amount );
	float cx = RainHeight( fuv + e.xy, t, amount );
	float cy = RainHeight( fuv + e.yx, t, amount );

	float2 g = float2( c - cx, c - cy );
	float3 N = normalize( float3( g * 30.0f, 1.0f ) );
	float edge = saturate( length( g ) * 25.0f );		// 水滴の縁.
	// 水滴のある所だけ表示する(しきい値で間引いて量を抑える).
	float wetMask = smoothstep( 0.25f, 0.6f, c );

	// 屈折(法線方向にずらして、ぼかしたガラス面をサンプリング).
	//	fuv は上下反転しているので、画面のY方向へは N.y を反転して適用する.
	//	水のある所ほどぼかしを弱めて鮮明にする(水滴がレンズになる).
	float2 refUV = i.UV + float2( N.x / aspect, -N.y ) * g_P0.z * 0.05f;
	float blurPx = g_P0.w * ( 1.0f - wetMask * 0.8f );
	float4 glass = GlassBlur( refUV, blurPx );

	// 背景が透明/暗い所では薄い水色にして黒くならないようにする.
	float3 tint = float3( 0.72f, 0.80f, 0.88f );
	float3 col = glass.rgb + tint * ( 1.0f - glass.a ) * wetMask;

	// 鏡面ハイライトと縁の明るさ.
	float3 lightDir = normalize( float3( -0.5f, -0.6f, 1.0f ) );
	float spec = pow( saturate( dot( N, lightDir ) ), 30.0f ) * g_P1.x * wetMask;
	col += edge * 0.06f * wetMask;
	col += spec;

	// 水に覆われた所はアルファを持ち上げる(透明ウィンドウでも見える).
	float a = max( src.a, max( glass.a, wetMask * saturate( 0.35f + edge * 0.6f + spec ) ) );
	return FinishColor( src, float4( saturate( col ), a ) );
}

//================================================.
//	雪(雪片オーバーレイ).
//	g_P0 : x:雪の量, y:落ちる速さ, z:雪片の大きさ, w:横揺れの強さ.
//	g_P1 : x:明るさ.
//================================================.
float4 PS_Snow( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float aspect = g_Common0.x / max( g_Common0.y, 1.0f );
	float mask = 0.0f;

	// 3層の雪を重ねる(奥の層ほど小さく遅い = 視差).
	[unroll]
	for ( int layer = 0; layer < 3; ++layer ) {
		float depth = 1.0f - layer * 0.3f;
		float scale = 8.0f + layer * 6.0f;			// セルの分割数.
		float2 p = float2( i.UV.x * aspect, i.UV.y ) * scale;
		// 層ごとに落下させる(UVは下が正なので減算で下へ流す).
		p.y -= g_Common0.z * g_P0.y * scale * ( 0.6f + 0.4f * depth );

		float2 id = floor( p );
		float2 f = frac( p );
		float rnd = Rand( id + layer * 17.0f );

		// セル内の雪片の位置(横に揺れながら落ちる).
		float2 flakePos;
		flakePos.x = 0.2f + 0.6f * Rand( id + 3.0f )
			+ sin( g_Common0.z * ( 0.5f + rnd ) + rnd * 6.28f ) * 0.2f * g_P0.w;
		flakePos.y = 0.2f + 0.6f * rnd;

		float radius = g_P0.z * ( 0.04f + 0.06f * Rand( id + 5.0f ) ) * depth;
		float dist = length( f - flakePos );
		float flake = smoothstep( radius, radius * 0.3f, dist );
		// 量の調整(セル単位で間引く).
		float active = step( 1.0f - g_P0.x, rnd );
		mask = max( mask, flake * active * depth );
	}

	float3 col = src.rgb + mask * g_P1.x;
	float a = max( src.a, mask * g_P1.x );
	return FinishColor( src, float4( saturate( col ), a ) );
}

//================================================.
//	フォグ(ノイズミスト式. 深度不要で2D/3D両用).
//	g_P0 : x:濃さ, y:粗さ, z:横の流れ, w:縦の流れ.
//	g_P1 : x:画面下部に溜まる度合い.
//	g_P2 : rgba:霧の色.
//================================================.
float4 PS_Fog( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float aspect = g_Common0.x / max( g_Common0.y, 1.0f );
	float2 p = float2( i.UV.x * aspect, i.UV.y ) * g_P0.y;

	// 2枚の流れる霧を重ねて自然なむらを作る.
	float2 flow = float2( g_P0.z, g_P0.w ) * g_Common0.z;
	float f1 = Fbm( p + flow );
	float f2 = Fbm( p * 1.4f - flow * 0.7f + 31.0f );
	float fog = f1 * 0.65f + f2 * 0.35f;

	// 画面下部に溜まる勾配.
	float gradient = lerp( 1.0f, smoothstep( 0.0f, 1.0f, i.UV.y ) * 1.6f, g_P1.x );

	float amount = saturate( ( fog - 0.25f ) * 1.8f * g_P0.x * gradient );
	float3 col = lerp( src.rgb, g_P2.rgb, amount );
	float a = max( src.a, amount );
	return FinishColor( src, float4( col, a ) );
}

//================================================.
//	深度フォグの共通処理.
//	depth : 深度バッファの値(0～1).
//================================================.
float4 FogDepthColor( float4 src, float depth )
{
	// 何も描画されていない部分(深度最大)にかけるか.
	if ( depth >= 0.999999f && g_P1.y < 0.5f ) return src;

	// 深度値をカメラからの距離へ変換する.
	float nearZ	= g_P0.x;
	float farZ	= g_P0.y;
	float viewZ	= nearZ * farZ / max( farZ - depth * ( farZ - nearZ ), 0.0001f );

	// 距離に応じたフォグの濃さ.
	float amount = saturate( ( viewZ - g_P0.z ) / max( g_P0.w - g_P0.z, 0.0001f ) ) * g_P1.x;
	float3 col = lerp( src.rgb, g_P2.rgb, amount );
	float a = max( src.a, amount * step( 0.999999f, depth ) );
	return float4( col, a );
}

//================================================.
//	深度フォグ(通常).
//	g_P0 : x:ニアクリップ, y:ファークリップ, z:開始距離, w:終了距離.
//	g_P1 : x:最大濃度, y:未描画部分にもかけるか.
//	g_P2 : rgba:フォグの色.
//================================================.
float4 PS_FogDepth( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float depth = g_DepthTexture.Load( int3( i.Pos.xy, 0 ) ).r;
	return FinishColor( src, FogDepthColor( src, depth ) );
}

//================================================.
//	深度フォグ(MSAA. 深度バッファがマルチサンプルの場合).
//================================================.
float4 PS_FogDepthMS( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float depth = g_DepthTextureMS.Load( int2( i.Pos.xy ), 0 ).r;
	return FinishColor( src, FogDepthColor( src, depth ) );
}
