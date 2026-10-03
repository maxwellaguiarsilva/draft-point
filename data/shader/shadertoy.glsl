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

layout( location = 0 ) uniform int sphere_count;
layout( location = 1 ) uniform vec2 resolution;

layout( std430, binding = 0 ) readonly buffer sphere_buffer { float sphere_data[]; };
layout( std430, binding = 1 ) readonly buffer environment_buffer { float environment_data[]; };

out vec4 final_color;

struct sphere
{
	vec3 position;
	float radius;
	vec4 color;
};

#define get_vec3_from( data, index ) vec3( data[ ( index ) ], data[ ( index ) + 1 ], data[ ( index ) + 2 ] )

sphere get_sphere( int index )
{
	int base_index = index * 8;
	return sphere(
		get_vec3_from( sphere_data, base_index + 0 ),
		sphere_data[ base_index + 3 ],
		vec4( sphere_data[ base_index + 4 ], sphere_data[ base_index + 5 ], sphere_data[ base_index + 6 ], sphere_data[ base_index + 7 ] )
	);
}

struct environment
{
	vec3 cam_position;
	vec3 cam_forward;
	vec3 cam_right;
	vec3 cam_up;
	vec4 background_color;
	float focal;
	float ambient;
	float volume;
};

environment get_environment( )
{
	return environment(
		get_vec3_from( environment_data, 0 ),
		get_vec3_from( environment_data, 3 ),
		get_vec3_from( environment_data, 6 ),
		get_vec3_from( environment_data, 9 ),
		vec4( environment_data[ 12 ], environment_data[ 13 ], environment_data[ 14 ], environment_data[ 15 ] ),
		environment_data[ 16 ],
		environment_data[ 17 ],
		environment_data[ 18 ]
	);
}

void main( )
{
	vec2 input_coord = ( gl_FragCoord.xy - 0.5 * resolution ) * ( 2.0 / min( resolution.x, resolution.y ) );

	const environment current_environment = get_environment( );

	vec3 direction = current_environment.cam_forward * current_environment.focal + current_environment.cam_right * input_coord.x + current_environment.cam_up * input_coord.y;

	float best_distance = 1e20;
	vec3 color = current_environment.background_color.rgb;

	for( int index = 0; index < sphere_count; ++index )
	{
		sphere object = get_sphere( index );
		vec3 hypotenuse = object.position - current_environment.cam_position;
		float dot_hypotenuse_direction = dot( hypotenuse, direction );
		float opposite_leg_squared = dot( hypotenuse, hypotenuse ) - ( dot_hypotenuse_direction * dot_hypotenuse_direction ) / dot( direction, direction );
		float ratio = opposite_leg_squared / ( object.radius * object.radius );
		float distance = length( hypotenuse );

		if( dot_hypotenuse_direction > 0.0 && distance < best_distance && ratio <= 1.0 )
		{
			best_distance = distance;
			color = object.color.rgb * ( current_environment.ambient + current_environment.volume * sqrt( 1.0 - clamp( ratio, 0.0, 1.0 ) ) );
		}
	}

	final_color = vec4( color, 1.0 );
}

//	fragment-shader
