package com.example.myapplication2

data class ResultStep(
    val stepIndex: Int,
    val p1: DoubleArray,
    val p2: DoubleArray,
    val pointIndex1: Int,
    val pointIndex2: Int,
    val pointList: List<Int>
)