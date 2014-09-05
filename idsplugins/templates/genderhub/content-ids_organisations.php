<?php
/**
 * The default template for displaying content. Used for both single and index/archive/search.
*/
?>

	<article id="post-<?php the_ID(); ?>" <?php post_class(); ?>>
		<?php if ( is_sticky() && is_home() && ! is_paged() ) : ?>
		<div class="featured-post">
			<?php _e( 'Featured post', 'twentythirteen' ); ?>
		</div>
		<?php endif; ?>
		<header class="entry-header">
			<?php the_post_thumbnail(); ?>
			<?php if ( is_single() ) : ?>
			<h1 class="entry-title"><?php the_title(); ?></h1>
			<?php else : ?>
			<h1 class="entry-title">
				<a href="<?php the_permalink(); ?>" title="<?php echo esc_attr( sprintf( __( 'Permalink to %s', 'twentythirteen' ), the_title_attribute( 'echo=0' ) ) ); ?>" rel="bookmark"><?php the_title(); ?></a>
			</h1>
			<?php endif; // is_single() ?>
			<?php if ( comments_open() ) : ?>
				<div class="comments-link">
					<?php comments_popup_link( '<span class="leave-reply">' . __( 'Leave a reply', 'twentythirteen' ) . '</span>', __( '1 Reply', 'twentythirteen' ), __( '% Replies', 'twentythirteen' ) ); ?>
				</div><!-- .comments-link -->
			<?php endif; // comments_open() ?>
		</header><!-- .entry-header -->

		<?php if ( is_search() ) : // Only display Excerpts for Search ?>
		<div class="entry-summary">
			<?php the_excerpt(); ?>
		</div><!-- .entry-summary -->
		<?php else : ?>
		<div class="entry-content">
			<?php the_content( __( 'Continue reading <span class="meta-nav">&rarr;</span>', 'twentythirteen' ) ); ?>

			<?php if ( is_single() ) : ?>
      <!-- Example of a simple way in which these fields can be displayed. They can be used in other parts of the templates. -->
      <div class="ids-fields entry-meta">
        <ul>
          <?php ids_acronym('<li class="ids-field">' . __('Acronym: ')); ?>
          <?php ids_organisation_url('<li class="ids-field">' . __('Organisation URL: ')); ?>
          <?php ids_organisation_type('<li class="ids-field">' . __('Organisation type: ')); ?>
          <?php ids_location_country('<li class="ids-field">' . __('Location country: ')); ?>
          <?php ids_date_updated('<li class="ids-field">' . __('Updated on: ')); ?>
          <?php ids_countries('<li class="ids-field">' . __('Countries: ')); ?>
          <?php ids_regions('<li class="ids-field">' . __('Regions: ')); ?>
          <?php ids_themes('<li class="ids-field">' . __('Themes: ')); ?>
        </ul>
      </div>
			<?php endif; // is_single() ?>

			<?php wp_link_pages( array( 'before' => '<div class="page-links">' . __( 'Pages:', 'twentythirteen' ), 'after' => '</div>' ) ); ?>
		</div><!-- .entry-content -->
		<?php endif; ?>

		<footer class="entry-meta">
			<?php twentythirteen_entry_meta(); ?>
      <!-- The edit link does not make sense with content retrieved with the API, so we comment it here. We can include it in single-ids_organisations.php for imported organisations -->
			<!--?php edit_post_link( __( 'Edit', 'twentythirteen' ), '<span class="edit-link">', '</span>' ); ?-->
			<?php if ( is_singular() && get_the_author_meta( 'description' ) && is_multi_author() ) : // If a user has filled out their description and this is a multi-author blog, show a bio on their entries. ?>
				<div class="author-info">
					<div class="author-avatar">
						<?php echo get_avatar( get_the_author_meta( 'user_email' ), apply_filters( 'twentythirteen_author_bio_avatar_size', 68 ) ); ?>
					</div><!-- .author-avatar -->
					<div class="author-description">
						<h2><?php printf( __( 'About %s', 'twentythirteen' ), get_the_author() ); ?></h2>
						<p><?php the_author_meta( 'description' ); ?></p>
						<div class="author-link">
							<a href="<?php echo esc_url( get_author_posts_url( get_the_author_meta( 'ID' ) ) ); ?>" rel="author">
								<?php printf( __( 'View all posts by %s <span class="meta-nav">&rarr;</span>', 'twentythirteen' ), get_the_author() ); ?>
							</a>
						</div><!-- .author-link	-->
					</div><!-- .author-description -->
				</div><!-- .author-info -->
			<?php endif; ?>
		</footer><!-- .entry-meta -->
	</article><!-- #post -->
