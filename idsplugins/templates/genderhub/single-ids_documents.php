<?php get_header(); ?>

<div class="section group main_content">
<div class="col span_3_of_4 padding10">

		<?php /* The loop */ ?>
			<?php while ( have_posts() ) : the_post(); ?>
      <!-- We include 'content-ids_documents' here, instead of 'content'. -->
			<?php get_template_part( 'content-ids_documents', get_post_format() ); ?>
							
									<?php comments_template(); ?>
									
									<nav class="nav-single">
							<h4 class="assistive-text"><?php _e( 'Navigate below to view more posts...', 'twentythirteen' ); ?></h3>
							<span class="nav-previous"><?php previous_post_link( '%link', '<span class="meta-nav">' . _x( '&larr;', 'Previous post link', 'twentythirteen' ) . '</span> %title' ); ?></span>
							<span class="nav-next"><?php next_post_link( '%link', '%title <span class="meta-nav">' . _x( '&rarr;', 'Next post link', 'twentythirteen' ) . '</span>' ); ?></span>
						</nav><!-- .nav-single -->

			

			<?php endwhile; ?>		 
		 
		

</div><!--/col span_3_of_4 padding10-->




<div class="col span_1_of_4 sidebar padding10">

<?php if ( function_exists( 'dynamic_sidebar' ) ) dynamic_sidebar( "news-sidebar" ); ?> 

</div><!--/col span_1_of_4-->


	
	

</div><!--/section group-->




<?php get_footer(); ?>