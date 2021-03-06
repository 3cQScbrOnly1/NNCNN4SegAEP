#ifndef SRC_ComputionGraph_H_
#define SRC_ComputionGraph_H_

#include "ModelParams.h"
#include "Instance.h"


// Each model consists of two parts, building neural graph and defining output losses.
struct ComputionGraph : Graph {
public:
	const static int MAX_SENTENCE_LENGTH = 2048;
	const static int MAX_ATT_SIZE = 10;
	const static int MAX_EVAL_SIZE = 10;
	const static int MAX_EVAL_LENGTH = 32;

public:
	// node instances
	vector<LookupNode> _word_inputs;
	WindowBuilder _word_window;
	vector<UniNode> _hidden;

	MaxPoolNode _word_max_pooling;


	vector<LookupNode> _att_inputs;
	Node _att_bucket;
	AvgPoolNode _att_avg_pooling;
	MaxPoolNode _att_max_pooling;
	MinPoolNode _att_min_pooling;
	ConcatNode _att_pooling_concat;

	vector<vector<LookupNode> > _eval_char_inputs;
	vector<WindowBuilder> _eval_char_windows;
	vector<vector<UniNode> > _eval_char_hiddens;
	vector<MaxPoolNode> _eval_char_max_poolings;
	vector<MinPoolNode> _eval_char_min_poolings;
	vector<AvgPoolNode> _eval_char_avg_poolings;
	vector<ConcatNode> _eval_char_pooling_concats;

	MaxPoolNode _eval_concat_max_pooling;
	MinPoolNode _eval_concat_min_pooling;
	AvgPoolNode _eval_concat_avg_pooling;
	Node _eval_concat_bucket;
	ConcatNode _eval_pooling_concat;

	LookupNode _polar_input;
	UniNode _polar_hidden;

	FourNode _concat_seg_att_eval_polar;

	LinearNode _output;
public:
	ComputionGraph() : Graph() {
	}

	~ComputionGraph() {
		clear();
	}

public:
	//allocate enough nodes 
	inline void createNodes(int sent_length, int att_size, int eval_size, int eval_length) {
		_word_inputs.resize(sent_length);
		_word_window.resize(sent_length);
		_hidden.resize(sent_length);
		_word_max_pooling.setParam(sent_length);

		_att_inputs.resize(att_size);
		_att_avg_pooling.setParam(att_size);
		_att_max_pooling.setParam(att_size);
		_att_min_pooling.setParam(att_size);

		_eval_char_inputs.resize(eval_size);
		_eval_char_windows.resize(eval_size);
		_eval_char_hiddens.resize(eval_size);
		_eval_char_max_poolings.resize(eval_size);
		_eval_char_min_poolings.resize(eval_size);
		_eval_char_avg_poolings.resize(eval_size);
		_eval_char_pooling_concats.resize(eval_size);
		for (int idx = 0; idx < eval_size; idx++) {
			_eval_char_inputs[idx].resize(eval_length);
			_eval_char_windows[idx].resize(eval_length);
			_eval_char_hiddens[idx].resize(eval_length);
			_eval_char_max_poolings[idx].setParam(eval_length);
			_eval_char_min_poolings[idx].setParam(eval_length);
			_eval_char_avg_poolings[idx].setParam(eval_length);
		}

		_eval_concat_max_pooling.setParam(eval_size);
		_eval_concat_min_pooling.setParam(eval_size);
		_eval_concat_avg_pooling.setParam(eval_size);

	}

	inline void clear() {
		Graph::clear();
		_word_inputs.clear();
		_att_inputs.clear();
		_word_window.clear();
		_hidden.clear();
	}

public:
	inline void initial(ModelParams& model, HyperParams& opts, AlignedMemoryPool* mem = NULL) {
		int max_word_size = _word_inputs.size();
		for (int idx = 0; idx < max_word_size; idx++) {
			_word_inputs[idx].setParam(&model.words);
			_word_inputs[idx].init(opts.wordDim, opts.dropProb, mem);
			_hidden[idx].setParam(&model.hidden_linear);
			_hidden[idx].init(opts.wordHiddenSize, opts.dropProb, mem);
			_hidden[idx].setFunctions(frelu, drelu);
		}
		_word_window.init(opts.wordDim, opts.wordContext, mem);
		_word_max_pooling.init(opts.wordHiddenSize, -1, mem);

		int max_att_size = _att_inputs.size();
		for (int idx = 0; idx < max_att_size; idx++) {
			_att_inputs[idx].setParam(&model.atts);
			_att_inputs[idx].init(opts.attDim, opts.dropProb, mem);
		}
		_att_bucket.set_bucket();
		_att_bucket.init(opts.attDim, -1, mem);
		_att_avg_pooling.init(opts.attDim, -1, mem);
		_att_max_pooling.init(opts.attDim, -1, mem);
		_att_min_pooling.init(opts.attDim, -1, mem);
		_att_pooling_concat.init(opts.attDim * 3, -1, mem);

		int max_eval_size = _eval_char_inputs.size();
		for (int idx = 0; idx < max_eval_size; idx++) {
			int max_eval_char_size = _eval_char_inputs[idx].size();
			for (int idy = 0; idy < max_eval_char_size; idy++) {
				_eval_char_inputs[idx][idy].setParam(&model.evalChars);
				_eval_char_inputs[idx][idy].init(opts.evalCharDim, opts.dropProb, mem);
				_eval_char_hiddens[idx][idy].setParam(&model.eval_char_hidden_linear);
				_eval_char_hiddens[idx][idy].init(opts.evalCharHiddenSize, opts.dropProb, mem);
			}
			_eval_char_windows[idx].init(opts.evalCharDim, opts.evalCharContext, mem);
			_eval_char_max_poolings[idx].init(opts.evalCharHiddenSize, -1, mem);
			_eval_char_min_poolings[idx].init(opts.evalCharHiddenSize, -1, mem);
			_eval_char_avg_poolings[idx].init(opts.evalCharHiddenSize, -1, mem);
			_eval_char_pooling_concats[idx].init(opts.evalCharHiddenSize * 3, -1, mem);
		}
		_eval_concat_max_pooling.init(opts.evalCharHiddenSize * 3, -1, mem);
		_eval_concat_min_pooling.init(opts.evalCharHiddenSize * 3, -1, mem);
		_eval_concat_avg_pooling.init(opts.evalCharHiddenSize * 3, -1, mem);
		_eval_concat_bucket.set_bucket();
		_eval_concat_bucket.init(opts.evalCharHiddenSize * 3, -1, mem);

		_eval_pooling_concat.init(opts.evalCharHiddenSize * 3 * 3, -1, mem);

		_polar_input.setParam(&model.polarity);
		_polar_input.init(opts.polarityDim, opts.polarDropProb, mem);
		_polar_hidden.setParam(&model.polar_hidden_linear);
		_polar_hidden.init(opts.polarityHiddenSize, opts.polarDropProb, mem);
		_polar_hidden.setFunctions(fsigmoid, dsigmoid);

		_concat_seg_att_eval_polar.setParam(&model.seg_att_eval_polar_concat);
		_concat_seg_att_eval_polar.init(opts.concatHiddenSize, opts.dropProb, mem);

		_output.setParam(&model.olayer_linear);
		_output.init(opts.labelSize, -1, mem);
	}


public:
	// some nodes may behave different during training and decode, for example, dropout
	inline void forward(const Instance& inst, bool bTrain = false) {
		//first step: clear value
		clearValue(bTrain); // compute is a must step for train, predict and cost computation

		// second step: build graph
		//forward
		int word_num = inst.m_segs.size();
		if (word_num > MAX_SENTENCE_LENGTH)
			word_num = MAX_SENTENCE_LENGTH;
		for (int i = 0; i < word_num; i++) {
			_word_inputs[i].forward(this, inst.m_segs[i]);
		}
		_word_window.forward(this, getPNodes(_word_inputs, word_num));

		for (int i = 0; i < word_num; i++) {
			_hidden[i].forward(this, &_word_window._outputs[i]);
		}
		_word_max_pooling.forward(this, getPNodes(_hidden, word_num));

		int att_num = inst.m_attributes.size();
		if (att_num > MAX_ATT_SIZE)
			att_num = MAX_ATT_SIZE;
		for (int i = 0; i < att_num; i++) {
			_att_inputs[i].forward(this, inst.m_attributes[i]);
		}

		if (att_num == 0)
			_att_pooling_concat.forward(this, &_att_bucket, &_att_bucket, &_att_bucket);
		else {
			_att_avg_pooling.forward(this, getPNodes(_att_inputs, att_num));
			_att_max_pooling.forward(this, getPNodes(_att_inputs, att_num));
			_att_min_pooling.forward(this, getPNodes(_att_inputs, att_num));
			_att_pooling_concat.forward(this, &_att_avg_pooling, &_att_max_pooling, &_att_min_pooling);
		}
		int eval_size = inst.m_eval_chars.size();
		if (eval_size > MAX_EVAL_SIZE)
			eval_size = MAX_EVAL_SIZE;
		int eval_length;
		for (int i = 0; i < eval_size; i++) {
			eval_length = inst.m_eval_chars[i].size();
			if (eval_length > MAX_EVAL_LENGTH)
				eval_length = MAX_EVAL_LENGTH;
			for (int j = 0; j < eval_length; j++) {
				_eval_char_inputs[i][j].forward(this, inst.m_eval_chars[i][j]);
			}
			_eval_char_windows[i].forward(this, getPNodes(_eval_char_inputs[i], eval_length));
			for (int j = 0; j < eval_length; j++) {
				_eval_char_hiddens[i][j].forward(this, &_eval_char_windows[i]._outputs[j]);
			}
			_eval_char_max_poolings[i].forward(this, getPNodes(_eval_char_hiddens[i], eval_length));
			_eval_char_min_poolings[i].forward(this, getPNodes(_eval_char_hiddens[i], eval_length));
			_eval_char_avg_poolings[i].forward(this, getPNodes(_eval_char_hiddens[i], eval_length));
			_eval_char_pooling_concats[i].forward(this, &_eval_char_max_poolings[i], &_eval_char_min_poolings[i], &_eval_char_avg_poolings[i]);
		}
		if (eval_size == 0)
			_eval_pooling_concat.forward(this, &_eval_concat_bucket, &_eval_concat_bucket, &_eval_concat_bucket);
		else {
			_eval_concat_max_pooling.forward(this, getPNodes(_eval_char_pooling_concats, eval_size));
			_eval_concat_min_pooling.forward(this, getPNodes(_eval_char_pooling_concats, eval_size));
			_eval_concat_avg_pooling.forward(this, getPNodes(_eval_char_pooling_concats, eval_size));
			_eval_pooling_concat.forward(this, &_eval_concat_max_pooling, &_eval_concat_min_pooling, &_eval_concat_avg_pooling);
		}

		_polar_input.forward(this, inst.m_polarity);
		_polar_hidden.forward(this, &_polar_input);
		_concat_seg_att_eval_polar.forward(this, &_word_max_pooling, &_att_pooling_concat, &_eval_pooling_concat, &_polar_hidden);

		_output.forward(this, &_concat_seg_att_eval_polar);
	}
};

#endif /* SRC_ComputionGraph_H_ */