#include "xdiff/xmacros.h"
#include "revision.h"

static int compare_paths(const struct combine_diff_path *one,
			  const struct diff_filespec *two)
{
	if (!S_ISDIR(one->mode) && !S_ISDIR(two->mode))
		return strcmp(one->path, two->path);

	return base_name_compare(one->path, strlen(one->path), one->mode,
				 two->path, strlen(two->path), two->mode);
}
	struct combine_diff_path *p, **tail = &curr;
	int i, cmp;
		return curr;
	/*
	 * paths in curr (linked list) and q->queue[] (array) are
	 * both sorted in the tree order.
	 */
	i = 0;
	while ((p = *tail) != NULL) {
		cmp = ((i >= q->nr)
		       ? -1 : compare_paths(p, q->queue[i]->two));

		if (cmp < 0) {
			/* p->path not in q->queue[]; drop it */
			*tail = p->next;
			free(p);
		}
		if (cmp > 0) {
			/* q->queue[i] not in p->path; skip it */
			i++;
			continue;

		hashcpy(p->parent[n].sha1, q->queue[i]->one->sha1);
		p->parent[n].mode = q->queue[i]->one->mode;
		p->parent[n].status = q->queue[i]->status;

		tail = &p->next;
		i++;
	struct lline *next, *prev;
/* Lines lost from current parent (before coalescing) */
struct plost {
	struct lline *lost_head, *lost_tail;
	int len;
};

	/* Accumulated and coalesced lost lines */
	struct lline *lost;
	int lenlost;
	struct plost plost;
static int match_string_spaces(const char *line1, int len1,
			       const char *line2, int len2,
			       long flags)
{
	if (flags & XDF_WHITESPACE_FLAGS) {
		for (; len1 > 0 && XDL_ISSPACE(line1[len1 - 1]); len1--);
		for (; len2 > 0 && XDL_ISSPACE(line2[len2 - 1]); len2--);
	}

	if (!(flags & (XDF_IGNORE_WHITESPACE | XDF_IGNORE_WHITESPACE_CHANGE)))
		return (len1 == len2 && !memcmp(line1, line2, len1));

	while (len1 > 0 && len2 > 0) {
		len1--;
		len2--;
		if (XDL_ISSPACE(line1[len1]) || XDL_ISSPACE(line2[len2])) {
			if ((flags & XDF_IGNORE_WHITESPACE_CHANGE) &&
			    (!XDL_ISSPACE(line1[len1]) || !XDL_ISSPACE(line2[len2])))
				return 0;

			for (; len1 > 0 && XDL_ISSPACE(line1[len1]); len1--);
			for (; len2 > 0 && XDL_ISSPACE(line2[len2]); len2--);
		}
		if (line1[len1] != line2[len2])
			return 0;
	}

	if (flags & XDF_IGNORE_WHITESPACE) {
		/* Consume remaining spaces */
		for (; len1 > 0 && XDL_ISSPACE(line1[len1 - 1]); len1--);
		for (; len2 > 0 && XDL_ISSPACE(line2[len2 - 1]); len2--);
	}

	/* We matched full line1 and line2 */
	if (!len1 && !len2)
		return 1;

	return 0;
}

enum coalesce_direction { MATCH, BASE, NEW };

/* Coalesce new lines into base by finding LCS */
static struct lline *coalesce_lines(struct lline *base, int *lenbase,
				    struct lline *new, int lennew,
				    unsigned long parent, long flags)
{
	int **lcs;
	enum coalesce_direction **directions;
	struct lline *baseend, *newend = NULL;
	int i, j, origbaselen = *lenbase;

	if (new == NULL)
		return base;

	if (base == NULL) {
		*lenbase = lennew;
		return new;
	}

	/*
	 * Coalesce new lines into base by finding the LCS
	 * - Create the table to run dynamic programming
	 * - Compute the LCS
	 * - Then reverse read the direction structure:
	 *   - If we have MATCH, assign parent to base flag, and consume
	 *   both baseend and newend
	 *   - Else if we have BASE, consume baseend
	 *   - Else if we have NEW, insert newend lline into base and
	 *   consume newend
	 */
	lcs = xcalloc(origbaselen + 1, sizeof(int*));
	directions = xcalloc(origbaselen + 1, sizeof(enum coalesce_direction*));
	for (i = 0; i < origbaselen + 1; i++) {
		lcs[i] = xcalloc(lennew + 1, sizeof(int));
		directions[i] = xcalloc(lennew + 1, sizeof(enum coalesce_direction));
		directions[i][0] = BASE;
	}
	for (j = 1; j < lennew + 1; j++)
		directions[0][j] = NEW;

	for (i = 1, baseend = base; i < origbaselen + 1; i++) {
		for (j = 1, newend = new; j < lennew + 1; j++) {
			if (match_string_spaces(baseend->line, baseend->len,
						newend->line, newend->len, flags)) {
				lcs[i][j] = lcs[i - 1][j - 1] + 1;
				directions[i][j] = MATCH;
			} else if (lcs[i][j - 1] >= lcs[i - 1][j]) {
				lcs[i][j] = lcs[i][j - 1];
				directions[i][j] = NEW;
			} else {
				lcs[i][j] = lcs[i - 1][j];
				directions[i][j] = BASE;
			}
			if (newend->next)
				newend = newend->next;
		}
		if (baseend->next)
			baseend = baseend->next;
	}

	for (i = 0; i < origbaselen + 1; i++)
		free(lcs[i]);
	free(lcs);

	/* At this point, baseend and newend point to the end of each lists */
	i--;
	j--;
	while (i != 0 || j != 0) {
		if (directions[i][j] == MATCH) {
			baseend->parent_map |= 1<<parent;
			baseend = baseend->prev;
			newend = newend->prev;
			i--;
			j--;
		} else if (directions[i][j] == NEW) {
			struct lline *lline;

			lline = newend;
			/* Remove lline from new list and update newend */
			if (lline->prev)
				lline->prev->next = lline->next;
			else
				new = lline->next;
			if (lline->next)
				lline->next->prev = lline->prev;

			newend = lline->prev;
			j--;

			/* Add lline to base list */
			if (baseend) {
				lline->next = baseend->next;
				lline->prev = baseend;
				if (lline->prev)
					lline->prev->next = lline;
			}
			else {
				lline->next = base;
				base = lline;
			}
			(*lenbase)++;

			if (lline->next)
				lline->next->prev = lline;

		} else {
			baseend = baseend->prev;
			i--;
		}
	}

	newend = new;
	while (newend) {
		struct lline *lline = newend;
		newend = newend->next;
		free(lline);
	}

	for (i = 0; i < origbaselen + 1; i++)
		free(directions[i]);
	free(directions);

	return base;
}

	lline->prev = sline->plost.lost_tail;
	if (lline->prev)
		lline->prev->next = lline;
	else
		sline->plost.lost_head = lline;
	sline->plost.lost_tail = lline;
	sline->plost.len++;
			 const char *path, long flags)
	xpp.flags = flags;
		/* Coalesce new lines */
		if (sline[lno].plost.lost_head) {
			struct sline *sl = &sline[lno];
			sl->lost = coalesce_lines(sl->lost, &sl->lenlost,
						  sl->plost.lost_head,
						  sl->plost.len, n, flags);
			sl->plost.lost_head = sl->plost.lost_tail = NULL;
			sl->plost.len = 0;
		}

		ll = sline[lno].lost;
	return ((sline->flag & all_mask) || sline->lost);
		while (j < i) {
			if (!(sline[j].flag & mark))
				sline[j].flag |= no_pre_delete;
			sline[j++].flag |= mark;
		}
			struct lline *ll = sline[j].lost;
static void dump_sline(struct sline *sline, const char *line_prefix,
		       unsigned long cnt, int num_parent,
	const char *c_context = diff_get_color(use_color, DIFF_CONTEXT);
		printf("%s%s", line_prefix, c_frag);
						    c_context, c_reset,
			ll = (sl->flag & no_pre_delete) ? NULL : sl->lost;
				printf("%s%s", line_prefix, c_old);
			fputs(line_prefix, stdout);
				fputs(c_context, stdout);
		struct lline *ll = sline->lost;
			     const char *line_prefix,
	strbuf_addstr(&buf, line_prefix);
				 const char *line_prefix,
			 "", elem->path, line_prefix, c_meta, c_reset);
	printf("%s%sindex ", line_prefix, c_meta);
			printf("%s%snew file mode %06o",
			       line_prefix, c_meta, elem->mode);
				printf("%s%sdeleted file ",
				       line_prefix, c_meta);
				 line_prefix, c_meta, c_reset);
				 line_prefix, c_meta, c_reset);
				 line_prefix, c_meta, c_reset);
				 line_prefix, c_meta, c_reset);
	const char *line_prefix = diff_line_prefix(opt);
				     line_prefix, mode_differs, 0);
				     textconv, elem->path, opt->xdl_opts);
				     line_prefix, mode_differs, 1);
		dump_sline(sline, line_prefix, cnt, num_parent,
		if (sline[lno].lost) {
			struct lline *ll = sline[lno].lost;
	int line_termination, inter_name_termination, i;
	const char *line_prefix = diff_line_prefix(opt);

		printf("%s", line_prefix);

		/* As many colons as there are parents */
		for (i = 0; i < num_parent; i++)
			putchar(':');
		for (i = 0; i < num_parent; i++)
			printf("%06o ", p->parent[i].mode);
		printf("%06o", p->mode);

	for (i = 0, p = paths; p; p = p->next)
static const char *path_path(void *obj)
{
	struct combine_diff_path *path = (struct combine_diff_path *)obj;

	return path->path;
}


/* find set of paths that every parent touches */
static struct combine_diff_path *find_paths_generic(const unsigned char *sha1,
	const struct sha1_array *parents, struct diff_options *opt)
{
	struct combine_diff_path *paths = NULL;
	int i, num_parent = parents->nr;

	int output_format = opt->output_format;
	const char *orderfile = opt->orderfile;

	opt->output_format = DIFF_FORMAT_NO_OUTPUT;
	/* tell diff_tree to emit paths in sorted (=tree) order */
	opt->orderfile = NULL;

	/* D(A,P1...Pn) = D(A,P1) ^ ... ^ D(A,Pn)  (wrt paths) */
	for (i = 0; i < num_parent; i++) {
		/*
		 * show stat against the first parent even when doing
		 * combined diff.
		 */
		int stat_opt = (output_format &
				(DIFF_FORMAT_NUMSTAT|DIFF_FORMAT_DIFFSTAT));
		if (i == 0 && stat_opt)
			opt->output_format = stat_opt;
		else
			opt->output_format = DIFF_FORMAT_NO_OUTPUT;
		diff_tree_sha1(parents->sha1[i], sha1, "", opt);
		diffcore_std(opt);
		paths = intersect_paths(paths, i, num_parent);

		/* if showing diff, show it in requested order */
		if (opt->output_format != DIFF_FORMAT_NO_OUTPUT &&
		    orderfile) {
			diffcore_order(orderfile);
		}

		diff_flush(opt);
	}

	opt->output_format = output_format;
	opt->orderfile = orderfile;
	return paths;
}


/*
 * find set of paths that everybody touches, assuming diff is run without
 * rename/copy detection, etc, comparing all trees simultaneously (= faster).
 */
static struct combine_diff_path *find_paths_multitree(
	const unsigned char *sha1, const struct sha1_array *parents,
	struct diff_options *opt)
{
	int i, nparent = parents->nr;
	const unsigned char **parents_sha1;
	struct combine_diff_path paths_head;
	struct strbuf base;

	parents_sha1 = xmalloc(nparent * sizeof(parents_sha1[0]));
	for (i = 0; i < nparent; i++)
		parents_sha1[i] = parents->sha1[i];

	/* fake list head, so worker can assume it is non-NULL */
	paths_head.next = NULL;

	strbuf_init(&base, PATH_MAX);
	diff_tree_paths(&paths_head, sha1, parents_sha1, nparent, &base, opt);

	strbuf_release(&base);
	free(parents_sha1);
	return paths_head.next;
}


	struct combine_diff_path *p, *paths;
	int need_generic_pathscan;

	/* nothing to do, if no parents */
	if (!num_parent)
		return;

	show_log_first = !!rev->loginfo && !rev->no_commit_id;
	needsep = 0;
	if (show_log_first) {
		show_log(rev);

		if (rev->verbose_header && opt->output_format &&
		    opt->output_format != DIFF_FORMAT_NO_OUTPUT &&
		    !commit_format_is_empty(rev->commit_format))
			printf("%s%c", diff_line_prefix(opt),
			       opt->line_termination);
	}
	copy_pathspec(&diffopts.pathspec, &opt->pathspec);
	/* find set of paths that everybody touches
	 *
	 * NOTE
	 *
	 * Diffcore transformations are bound to diff_filespec and logic
	 * comparing two entries - i.e. they do not apply directly to combine
	 * diff.
	 *
	 * If some of such transformations is requested - we launch generic
	 * path scanning, which works significantly slower compared to
	 * simultaneous all-trees-in-one-go scan in find_paths_multitree().
	 *
	 * TODO some of the filters could be ported to work on
	 * combine_diff_paths - i.e. all functionality that skips paths, so in
	 * theory, we could end up having only multitree path scanning.
	 *
	 * NOTE please keep this semantically in sync with diffcore_std()
	 */
	need_generic_pathscan = opt->skip_stat_unmatch	||
			DIFF_OPT_TST(opt, FOLLOW_RENAMES)	||
			opt->break_opt != -1	||
			opt->detect_rename	||
			opt->pickaxe		||
			opt->filter;


	if (need_generic_pathscan) {
		/*
		 * NOTE generic case also handles --stat, as it computes
		 * diff(sha1,parent_i) for all i to do the job, specifically
		 * for parent0.
		 */
		paths = find_paths_generic(sha1, parents, &diffopts);
	}
	else {
		int stat_opt;
		paths = find_paths_multitree(sha1, parents, &diffopts);

		/*
		 * show stat against the first parent even
		stat_opt = (opt->output_format &
		if (stat_opt) {
			diff_tree_sha1(parents->sha1[0], sha1, "", &diffopts);
			diffcore_std(&diffopts);
			if (opt->orderfile)
				diffcore_order(opt->orderfile);
			diff_flush(&diffopts);
	/* find out number of surviving paths */
	for (num_paths = 0, p = paths; p; p = p->next)
		num_paths++;

	/* order paths according to diffcore_order */
	if (opt->orderfile && num_paths) {
		struct obj_order *o;

		o = xmalloc(sizeof(*o) * num_paths);
		for (i = 0, p = paths; p; p = p->next, i++)
			o[i].obj = p;
		order_objects(opt->orderfile, path_path, o, num_paths);
		for (i = 0; i < num_paths - 1; i++) {
			p = o[i].obj;
			p->next = o[i+1].obj;
		}

		p = o[num_paths-1].obj;
		p->next = NULL;
		paths = o[0].obj;
		free(o);


			for (p = paths; p; p = p->next)
				show_raw_diff(p, num_parent, rev);
				printf("%s%c", diff_line_prefix(opt),
				       opt->line_termination);
			for (p = paths; p; p = p->next)
				show_patch_diff(p, num_parent, dense,
						0, rev);

	free_pathspec(&diffopts.pathspec);
	struct commit_list *parent = get_saved_parents(rev, commit);