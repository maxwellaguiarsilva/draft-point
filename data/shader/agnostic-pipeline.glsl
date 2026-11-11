//	vertex-shader
#version 460 core

const vec2 vertices[ 4 ] = vec2[ 4 ](
	 vec2( -1.0, -1.0 )
	,vec2(  1.0, -1.0 )
	,vec2( -1.0,  1.0 )
	,vec2(  1.0,  1.0 )
);

void main( ) { gl_Position = vec4( vertices[ gl_VertexID ], 0.0, 1.0 ); }
//	vertex-shader

//	fragment-shader
#version 460 core

layout( std430, binding = 0 ) readonly buffer environment_buffer { uint count; uint stride; float data[]; } environment_ssbo;
layout( std430, binding = 1 ) readonly buffer camera_buffer { uint count; uint stride; float data[]; } camera_ssbo;
layout( std430, binding = 2 ) readonly buffer figure_buffer { uint count; uint stride; float data[]; } figure_ssbo;

out vec4 final_color;

#define vec2_from( data, index ) vec2( data[ ( index ) ], data[ ( index ) + 1u ] )
#define vec3_from( data, index ) vec3( data[ ( index ) ], data[ ( index ) + 1u ], data[ ( index ) + 2u ] )
#define vec4_from( data, index ) vec4( data[ ( index ) ], data[ ( index ) + 1u ], data[ ( index ) + 2u ], data[ ( index ) + 3u ] )
#define square( value ) ( ( value ) * ( value ) )

struct environment
{
	vec4	background_color;
	float	focal;
	float	ambient;
	float	volume;
	vec2	half_screen_size;
	float	coordinate_scale;
};

environment ssbo_environment( uint index )
{
	uint	base_index = index * environment_ssbo.stride;
	return	environment(
		 vec4_from( environment_ssbo.data, base_index + 0u )
		,environment_ssbo.data[ base_index + 4u ]
		,environment_ssbo.data[ base_index + 5u ]
		,environment_ssbo.data[ base_index + 6u ]
		,vec2_from( environment_ssbo.data, base_index + 7u )
		,environment_ssbo.data[ base_index + 9u ]
	);
}

struct camera
{
	vec3	position;
	vec3	forward;
	vec3	right;
	vec3	up;
};

camera ssbo_camera( uint index )
{
	uint	base_index = index * camera_ssbo.stride;
	return	camera(
		 vec3_from( camera_ssbo.data, base_index + 0u )
		,vec3_from( camera_ssbo.data, base_index + 3u )
		,vec3_from( camera_ssbo.data, base_index + 6u )
		,vec3_from( camera_ssbo.data, base_index + 9u )
	);
}

//	@figure_parameter_capacity
struct figure
{
	vec3	position;
	float	type_id;
	vec4	color;
	float	parameters[ figure_parameter_capacity ];
};

figure ssbo_figure( uint index )
{
	uint	base_index = index * figure_ssbo.stride;
	figure	figure_item;
	figure_item.position = vec3_from( figure_ssbo.data, base_index + 0u );
	figure_item.type_id = figure_ssbo.data[ base_index + 3u ];
	figure_item.color = vec4_from( figure_ssbo.data, base_index + 4u );
	for( uint slot = 0u; slot < figure_parameter_capacity; ++slot )
		figure_item.parameters[ slot ] = figure_ssbo.data[ base_index + 8u + slot ];
	return	figure_item;
}

//	horizontal reductions and cyclic shifts isolated in the fragment template prelude
vec3 rotated_1( vec3 value ) { return value.yzx; }
vec3 rotated_2( vec3 value ) { return value.zxy; }
float min_axis( vec3 value ) { return min( value.x, min( value.y, value.z ) ); }
float max_axis( vec3 value ) { return max( value.x, max( value.y, value.z ) ); }
float sum_axis( vec3 value ) { return dot( value, vec3( 1.0 ) ); }

struct hit_result
{
	bool	hit;
	float	hit_distance;
	float	cos_incidence;
};

//	@figure_functions
hit_result dispatch_figure( vec3 ray_origin, vec3 unit_direction, figure figure_item )
{
//	@figure_dispatch
	return	hit_result( false, -1.0, 0.0 );
}

void main( )
{
	const	environment	s_environment = ssbo_environment( 0u );
	vec2	input_coord = ( gl_FragCoord.xy - s_environment.half_screen_size ) * s_environment.coordinate_scale;

	const	camera	s_camera = ssbo_camera( 0u );
	vec3	direction = s_camera.forward * s_environment.focal + s_camera.right * input_coord.x + s_camera.up * input_coord.y;
	vec3	unit_direction = normalize( direction );
	float	best_distance = 1e20;
	vec3	color = s_environment.background_color.rgb;

	for( uint index = 0u; index < figure_ssbo.count; ++index )
	{
		figure	figure_item = ssbo_figure( index );
		hit_result	candidate = dispatch_figure( s_camera.position, unit_direction, figure_item );
		if( candidate.hit && candidate.hit_distance > 0.0 && candidate.hit_distance < best_distance )
		{
			best_distance = candidate.hit_distance;
			color = figure_item.color.rgb * ( s_environment.ambient + s_environment.volume * candidate.cos_incidence );
		}
	}

	final_color = vec4( color, 1.0 );
}

//	fragment-shader
