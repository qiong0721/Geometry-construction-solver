
package com.example.myapplication2

class Solver {
    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }

    external fun solveGeometryProblem(
        initPoints: DoubleArray,
        initPointCount: Int,
        initLines: DoubleArray,
        initLineCount: Int,
        initCircles: DoubleArray,
        initCircleCount: Int,
        targetPoints: DoubleArray,
        targetLine: DoubleArray,
        targetCircle: DoubleArray,
        mode: String,
        stepLimit: Int,
        modenote: Int,
        goalNote: Int
    ): Array<ResultStep>
}