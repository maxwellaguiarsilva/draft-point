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

layout( location = 1 ) uniform vec2 resolution;

layout( std430, binding = 0 ) readonly buffer environment_buffer { uint count; uint stride; float data[]; } environment_ssbo;
layout( std430, binding = 1 ) readonly buffer camera_buffer { uint count; uint stride; float data[]; } camera_ssbo;
layout( std430, binding = 2 ) readonly buffer sphere_buffer { uint count; uint stride; float data[]; } sphere_ssbo;

out vec4 final_color;

#define vec3_from( data, index ) vec3( data[ ( index ) ], data[ ( index ) + 1u ], data[ ( index ) + 2u ] )
#define vec4_from( data, index ) vec4( data[ ( index ) ], data[ ( index ) + 1u ], data[ ( index ) + 2u ], data[ ( index ) + 3u ] )
#define square( value ) ( ( value ) * ( value ) )

struct environment
{
	vec4	background_color;
	float	focal;
	float	ambient;
	float	volume;
};

environment ssbo_environment( uint index )
{
	uint	base_index = index * environment_ssbo.stride;
	return	environment(
		 vec4_from( environment_ssbo.data, base_index + 0u )
		,environment_ssbo.data[ base_index + 4u ]
		,environment_ssbo.data[ base_index + 5u ]
		,environment_ssbo.data[ base_index + 6u ]
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

struct sphere
{
	vec3	position;
	float	radius;
	vec4	color;
};

sphere ssbo_sphere( uint index )
{
	uint	base_index = index * sphere_ssbo.stride;
	return	sphere(
		 vec3_from( sphere_ssbo.data, base_index + 0u )
		,sphere_ssbo.data[ base_index + 3u ]
		,vec4_from( sphere_ssbo.data, base_index + 4u )
	);
}

void main( )
{
	vec2	input_coord = ( gl_FragCoord.xy - 0.5 * resolution ) * ( 2.0 / min( resolution.x, resolution.y ) );

	const	environment	s_environment = ssbo_environment( 0u );
	const	camera	s_camera = ssbo_camera( 0u );
	vec3	direction = s_camera.forward * s_environment.focal + s_camera.right * input_coord.x + s_camera.up * input_coord.y;
	float	best_distance = 1e20;
	vec3	color = s_environment.background_color.rgb;

	for( uint index = 0u; index < sphere_ssbo.count; ++index )
	{
		sphere	s_sphere = ssbo_sphere( index );
		vec3	hypotenuse = s_sphere.position - s_camera.position;
		float	dot_hypotenuse_direction = dot( hypotenuse, direction );
		if( dot_hypotenuse_direction <= 0.0 ) continue;
		float	opposite_leg_squared = dot( hypotenuse, hypotenuse ) - square( dot_hypotenuse_direction ) / dot( direction, direction );
		float	ratio = opposite_leg_squared / square( s_sphere.radius );
		float	distance = length( hypotenuse );

		if( distance < best_distance && ratio <= 1.0 )
		{
			best_distance = distance;
			color = s_sphere.color.rgb * ( s_environment.ambient + s_environment.volume * sqrt( 1.0 - clamp( ratio, 0.0, 1.0 ) ) );
		}
	}

	final_color = vec4( color, 1.0 );
}

//	fragment-shader
